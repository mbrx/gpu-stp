/** \file opencl.h
 */
/* Copyright (C) 2013 Mathias Broxvall

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

#ifndef OPENCL_H_
#define OPENCL_H_

#include "CL/cl.h"

#define assertCLSuccess(err,dev,msg) OpenCL::assertSuccess(err,dev,msg,__FILE__,__LINE__)

class OpenCL {
 public:
  /** Compiles an OpenCL program on behalf of another concrete class.
   * Returns OpenCL program object from which actual kernels should be extracted. */
  cl_program compileProgram(int dev, const char *sourceFileName, const char *compileArgument);

  static void initCL(int *platformSelections, int *deviceSelections, int nSelections);
  static void cleanupCL();
  static class OpenCL *openCL;

  static void assertSuccess(cl_int err, int dev, const char *msg, const char *file, int line);
  static const char *errorString(int error);

  static const int nQueues=2;
  /** Structure to contain each device/platform combination that have been selected, and any command queues and other data required for these devices. */
  struct ComputeDevice {
    char name[256];

    cl_platform_id platform;
    cl_device_id device;
    cl_device_type deviceType;
    cl_context context;
    cl_command_queue queues[nQueues];
  };
  struct ComputeDevice devices[8];
  int nDevices;

  static const int cMaxPlatforms;
 private:
  OpenCL(int *platformSelections, int *deviceSelections, int nSelections);
  virtual ~OpenCL();
};

#endif /* OPENCL_H_ */
