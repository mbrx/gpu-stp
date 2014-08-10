/** \file clsolver.cc
 */
/* Copyright (C) 2014 Mathias Broxvall

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "general.h"
#include "CL/cl_platform.h"
#include "CL/cl.h"
#include "opencl.h"
#include "clsolver.h"
#include "sys/time.h"

CLSolver::CLSolver(int maxProblemSize) :maxProblemSize(maxProblemSize) {
  int dev;
  OpenCL *openCL = OpenCL::openCL;
  cl_int err;  
  int totalSize;

  for(dev=0;dev<openCL->nDevices;dev++) {
     char extraArgs[1024];
     snprintf(extraArgs, sizeof(extraArgs), "");
     // Also: try -g -O0 when debugging
    
     devices[dev].program = openCL->compileProgram(dev, "stp.cl", extraArgs);

     /* Don't kill me... syntactic sugar taken from a much much larger project of mine where it made sense*/

#define LoadKernel(name) { \
        devices[dev]. kern_##name = clCreateKernel(devices[dev].program, #name, &err); \
        assertCLSuccess(err, dev, "loading kernel " #name); \
}
#define CreateEvent(name) { \
        devices[dev]. name = clCreateUserEvent(openCL->devices[dev].context, &err); \
        assertCLSuccess(err, dev, "Creating event " #name); \
}
#define CreateBuffer(name,size,type) { \
        devices[dev]. name = clCreateBuffer(openCL->devices[dev].context, CL_MEM_##type, size, NULL, &err); \
        printf("Creating buffer %s of %lld MiB\n",#name,(long long int)((size)/(1024*1024))); \
        totalSize += (size); \
        assertCLSuccess(err, dev, "buffer " #name); \
}

     LoadKernel(basicSTP);
     CreateBuffer(buf_stp,maxProblemSize*maxProblemSize*sizeof(float),READ_WRITE);
     CreateBuffer(buf_flag,sizeof(int)*1,READ_WRITE);
     CreateEvent(event_basicSTP_done);    
  }
}
CLSolver::~CLSolver() {
  OpenCL *openCL = OpenCL::openCL;
  for (int dev = 0; dev<openCL->nDevices; dev++) {
    printf("Cleaning up device %d\n", dev);
    clReleaseMemObject(devices[dev].buf_stp);
    clReleaseMemObject(devices[dev].buf_flag);
    clReleaseKernel(devices[dev].kern_basicSTP);
    // TODO: release the event?
  }
}

double gettimef() {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return tv.tv_sec + tv.tv_usec * 1e-6;
  
}

int CLSolver::solve(int N, float *hostSTP) {
  // TODO: allow multiple devices to be used simultaneously
  int dev=0;

  cl_int err;  
  OpenCL *openCL = OpenCL::openCL;
  CLSolver::ComputeDeviceData &device = devices[dev];
  cl_command_queue queue = openCL->devices[dev].queues[0];

  double t0 = gettimef();
  clEnqueueWriteBuffer(queue,device.buf_stp, CL_TRUE, 0, sizeof(float)*N*N, hostSTP, 0, NULL, NULL);   

  cl_int argN = N;
  err = clSetKernelArg(device.kern_basicSTP, 0, sizeof(cl_int), &argN);
  assertCLSuccess(err, dev, "solve setArg 0");

  err = clSetKernelArg(device.kern_basicSTP, 2, sizeof(cl_mem), &device.buf_stp);
  assertCLSuccess(err, dev, "solve setArg 2");

  size_t globalSize[1] = { N };
  size_t workgroupSize[1] = { 256 };

  for(int k=0;k<N;k++) {
    cl_int argK = k;
    err = clSetKernelArg(device.kern_basicSTP, 1, sizeof(cl_int), &argK);
    assertCLSuccess(err, dev, "solve setArg 1");   
    err=clEnqueueNDRangeKernel(queue,device.kern_basicSTP, 1, NULL, globalSize, workgroupSize, 0, NULL, NULL);
    assertCLSuccess(err, dev, "solve run kern_basicSTP");
  }

  clEnqueueReadBuffer(queue,device.buf_stp, CL_TRUE, 0, sizeof(float)*N*N, hostSTP, 0, NULL,  &device.event_basicSTP_done); 
  clWaitForEvents(1,&device.event_basicSTP_done);

  double t1 = gettimef();
  printf("%d %f\n",N,t1-t0);
}

