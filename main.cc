/** \file main.cc
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

#include "general.h"
#include "opencl.h"
#include "clsolver.h"

#define _ISOC99_SOURCE
#include <math.h>

void printHelp(char *programName) {
  printf("USAGE: %s [options]"
	 "           --use <plat> <dev>  Select platforms and devices to use\n"
	 , programName
	 );
  exit(0);
}

int main(int argc, char **argv) {
  int i;
  int platformSelections[16], deviceSelections[16], nSelections;
  nSelections = 0;

  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--use") == 0) {
      platformSelections[nSelections] = atoi(argv[++i]);
      deviceSelections[nSelections++] = atoi(argv[++i]);
    } else
      printHelp(argv[0]);
    /* Add more commandline options here */
  }
  OpenCL::initCL(platformSelections, deviceSelections, nSelections);

  /* The representation of a single problem instance is a square array of floats. 
     Semantics:
     x_i <= x_j + array[j][i]
  */

  float *myProblem = new float[8192*8192];  
  CLSolver *solver = new CLSolver(8192);

  /* The actual time to solve the problem is neglibly affected by the constrainedness of it. 
     So for these benchmarks we are using just uninitialized memory. See below for real examples. */
  for(int N=0;N<=4096;N+=512)
    solver->solve(N,myProblem);


  /*
  for(int sample=0;sample<1;sample++) { 
    int N=2048;
#define C(i,j) myProblem[i+j*N]
    for(int i=0;i<N;i++) 
      for(int j=0;j<N;j++) {
	if(i == j) C(i,j) = 0.0;
	else C(i,j) = INFINITY;
      }       
    for(int constraint=0;constraint<100*sample;constraint++) {
      int i=rand() % N;
      int j=rand() % N;
      C(i,j) = ((rand() % 10000)-5000)/3.0;
    }    
    solver->solve(N,myProblem);
    printf("C(0,0): %f\n",C(0,0));
  }
  */

  delete solver;
  OpenCL::cleanupCL();
}

