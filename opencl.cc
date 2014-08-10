/** \file OpenCL.cc
 */
/* Copyright (C) 2013- Mathias Broxvall

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
#include <unistd.h>

/* Singleton instance used for all computations */
OpenCL *OpenCL::openCL;
const int OpenCL::cMaxPlatforms = 8;

const char *OpenCL::errorString(int code) {
  switch (code) {
  case CL_SUCCESS:
    return "Success!";
  case CL_DEVICE_NOT_FOUND:
    return "Device not found.";
  case CL_DEVICE_NOT_AVAILABLE:
    return "Device not available";
  case CL_COMPILER_NOT_AVAILABLE:
    return "Compiler not available";
  case CL_MEM_OBJECT_ALLOCATION_FAILURE:
    return "Memory object allocation failure";
  case CL_OUT_OF_RESOURCES:
    return "Out of resources";
  case CL_OUT_OF_HOST_MEMORY:
    return "Out of host memory";
  case CL_PROFILING_INFO_NOT_AVAILABLE:
    return "Profiling information not available";
  case CL_MEM_COPY_OVERLAP:
    return "Memory copy overlap";
  case CL_IMAGE_FORMAT_MISMATCH:
    return "Image format mismatch";
  case CL_IMAGE_FORMAT_NOT_SUPPORTED:
    return "Image format not supported";
  case CL_BUILD_PROGRAM_FAILURE:
    return "Program build failure";
  case CL_MAP_FAILURE:
    return "Map failure";
  case CL_INVALID_VALUE:
    return "Invalid value";
  case CL_INVALID_DEVICE_TYPE:
    return "Invalid device type";
  case CL_INVALID_PLATFORM:
    return "Invalid platform";
  case CL_INVALID_DEVICE:
    return "Invalid device";
  case CL_INVALID_CONTEXT:
    return "Invalid context";
  case CL_INVALID_QUEUE_PROPERTIES:
    return "Invalid queue properties";
  case CL_INVALID_COMMAND_QUEUE:
    return "Invalid command queue";
  case CL_INVALID_HOST_PTR:
    return "Invalid host pointer";
  case CL_INVALID_MEM_OBJECT:
    return "Invalid memory object";
  case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
    return "Invalid image format descriptor";
  case CL_INVALID_IMAGE_SIZE:
    return "Invalid image size";
  case CL_INVALID_SAMPLER:
    return "Invalid sampler";
  case CL_INVALID_BINARY:
    return "Invalid binary";
  case CL_INVALID_BUILD_OPTIONS:
    return "Invalid build options";
  case CL_INVALID_PROGRAM:
    return "Invalid program";
  case CL_INVALID_PROGRAM_EXECUTABLE:
    return "Invalid program executable";
  case CL_INVALID_KERNEL_NAME:
    return "Invalid kernel name";
  case CL_INVALID_KERNEL_DEFINITION:
    return "Invalid kernel definition";
  case CL_INVALID_KERNEL:
    return "Invalid kernel";
  case CL_INVALID_ARG_INDEX:
    return "Invalid argument index";
  case CL_INVALID_ARG_VALUE:
    return "Invalid argument value";
  case CL_INVALID_ARG_SIZE:
    return "Invalid argument size";
  case CL_INVALID_KERNEL_ARGS:
    return "Invalid kernel arguments";
  case CL_INVALID_WORK_DIMENSION:
    return "Invalid work dimension";
  case CL_INVALID_WORK_GROUP_SIZE:
    return "Invalid work group size";
  case CL_INVALID_WORK_ITEM_SIZE:
    return "Invalid work item size";
  case CL_INVALID_GLOBAL_OFFSET:
    return "Invalid global offset";
  case CL_INVALID_EVENT_WAIT_LIST:
    return "Invalid event wait list";
  case CL_INVALID_EVENT:
    return "Invalid event";
  case CL_INVALID_OPERATION:
    return "Invalid operation";
  case CL_INVALID_GL_OBJECT:
    return "Invalid OpenGL object";
  case CL_INVALID_BUFFER_SIZE:
    return "Invalid buffer size";
  case CL_INVALID_MIP_LEVEL:
    return "Invalid mip-map level";
  default:
    return "Unknown";
  }
}
void OpenCL::initCL(int *platformSelections, int *deviceSelections, int nSelections) {
  new OpenCL(platformSelections, deviceSelections, nSelections);
}
void openclErrorCallback(const char *errinfo, const void *privateInfo, size_t cb, void *userData) {
  printf("Opencl error: %s\n", errinfo);
}
void OpenCL::assertSuccess(cl_int err, int dev, const char *msg, const char *file, int line) {
  if (dev < 0 || dev > OpenCL::openCL->nDevices) {
    fprintf(stderr, "Error %s:%d: Attempting to reference invalid device %d - %s\n", file, line, dev, msg);
    exit(-1);
  }
  if (err) {
    fprintf(stderr, "[ERROR] %s:%d: OpenCL %s [dev %s] - %s\n", file, line, errorString(err), OpenCL::openCL->devices[dev].name, msg);
    exit(-1);
  }
}

OpenCL::OpenCL(int *platformSelections, int *deviceSelections, int nSelections) {
  cl_int err;
  int i, j, k;

  // Initialize singleton as early as possible since it is used during it's creation
  OpenCL::openCL = this;

  cl_platform_id platforms[cMaxPlatforms];
  cl_uint numPlatforms;
  err = clGetPlatformIDs(cMaxPlatforms, platforms, &numPlatforms);
  if (err != CL_SUCCESS) {
    fprintf(stderr, "Failed to identify OpenCL platforms. Problem with OpenCL installation/drivers?\n");
    exit(0);
  }
  printf("Found %d OpenCL platforms\n", numPlatforms);
  nDevices = 0;
  for (i = 0; i < (int) numPlatforms; i++) {
    char name[256], vendor[256];
    clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, sizeof(name), name, NULL);
    clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, sizeof(vendor), vendor, NULL);
    printf("Platform %d: name='%s' vendor='%s'\n", i, name, vendor);

    const cl_uint maxPlatformDevices = 8;
    cl_device_id platformDevices[maxPlatformDevices];
    cl_uint numPlatformDevices;
    err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, maxPlatformDevices, platformDevices, &numPlatformDevices);
    if (err != CL_SUCCESS) {
      fprintf(stderr, "Error querying platform %d for devices\n", i);
      continue;
    }
    printf("  Platform has %d devices:\n", numPlatformDevices);
    for (j = 0; j < (int) numPlatformDevices; j++) {
      cl_device_type type;
      cl_ulong maxAllocSize, globalSize;
      char deviceName[256];
      clGetDeviceInfo(platformDevices[j], CL_DEVICE_TYPE, sizeof(type), &type, NULL);
      clGetDeviceInfo(platformDevices[j], CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(&maxAllocSize), &maxAllocSize, NULL);
      clGetDeviceInfo(platformDevices[j], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(&globalSize), &globalSize, NULL);
      clGetDeviceInfo(platformDevices[j], CL_DEVICE_NAME, sizeof(deviceName), deviceName, NULL);
      printf("Device %d.%d: name='%s' type=%d (%s) maxAllocSize=%dM globalMemSize=%dM\n", i, j, deviceName, (int) type,
	     type == CL_DEVICE_TYPE_CPU ? "cpu" : (type == CL_DEVICE_TYPE_GPU ? "gpu" : "??"), (int) (maxAllocSize / (1024 * 1024)),
	     (int) (globalSize / (1024 * 1024)));

      for (k = 0; k < nSelections; k++) {
	if (i == platformSelections[k] && j == deviceSelections[k]) {
	  devices[nDevices].platform = platforms[j];
	  devices[nDevices].device = platformDevices[j];
	  snprintf(devices[nDevices].name, 255, "%d.%d: %s", i, j, deviceName);
	  devices[nDevices].deviceType = type;
	  nDevices++;
	}
      }
    }

  }
  if (nDevices < nSelections)
    fprintf(stderr, "[WARN] Some selected devices not found\n");
  if (nDevices < 1) {
    fprintf(stderr, "[ERROR] No devices found\n");
    exit(-1);
  }

  for (int dev = 0; dev < nDevices; dev++) {
    printf("Preparing context for %s\n", devices[dev].name);

    /* Create a unique context for this specific device */
    devices[dev].context = clCreateContext(NULL, 1, &devices[dev].device, openclErrorCallback, (void*) &devices[dev], &err);
    assertCLSuccess(err, dev, "Context creation");
    /* Create command queue for this device */
    int flags=0;
    //int flags=CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE;
    for(int i=0;i<nQueues;i++) {
      devices[dev].queues[i] = clCreateCommandQueue(devices[dev].context, devices[dev].device, flags, &err);
      assertCLSuccess(err, dev, "Command queue creation");
    }
  }
  printf("[OK] OpenCL initialization successfull\n");
}
OpenCL::~OpenCL() {
  printf("Cleaing OpenCL devices\n");
  for (int dev = 0; dev < nDevices; dev++) {
    for(int n=0;n<nQueues;n++)
      clReleaseCommandQueue(devices[dev].queues[n]);
    clReleaseContext(devices[dev].context);
    clReleaseDevice(devices[dev].device);
  }
}
void OpenCL::cleanupCL() {
  printf("Cleaning opencl\n");
  delete openCL;
  openCL=NULL;
}
cl_program OpenCL::compileProgram(int dev, const char *sourceFileName, const char *compileArguments) {
  cl_int err, err2;
  cl_int status;

  char the_path[256];
  getcwd(the_path, 255);
  char defines[2048];
  sprintf(defines, "-I./ -I%s -w %s", the_path, compileArguments);
  printf("Compiling '%s' for %s\n", sourceFileName, devices[dev].name);

  fflush(stdout);
  char sourceData[1024];
  sprintf(sourceData, "#include \"%s\"\n", sourceFileName);
  const char *sources[2] = { sourceData, NULL };
  cl_program program = clCreateProgramWithSource(devices[dev].context, 1, sources, NULL, &err);
  err = clBuildProgram(program, 1, &devices[dev].device, defines, NULL, NULL);
  if (err) {
    printf("Compilation arguments:\n%s\n", defines);
    char logtmp[4096];
    err2 = clGetProgramBuildInfo(program, devices[dev].device, CL_PROGRAM_BUILD_STATUS, sizeof(status), (void*) &status, NULL);
    printf("Build status: %d (get buildInfo return value %d)\n", status, err2);
    err2 = clGetProgramBuildInfo(program, devices[dev].device, CL_PROGRAM_BUILD_LOG, sizeof(logtmp), (void*) logtmp, NULL);
    printf("Build log (%d):\n%s\n", err2, logtmp);
    assertCLSuccess(err, dev, "Compiling kernels");
  }
  printf("[OK] Compilation successfull\n");
  return program;
}
