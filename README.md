gpu-stp
=======

A GPU/OpenCL proof of concept for Simple Temporal Problems (STP)

usage
=====

First install OpenCL, then run the program once without args. It will print
the ID of your available OpenCL platforms. Pick one and run it again
with the corresponding --use X Y numbers.

For a real application you should (a) use multiple queues to
parallelise the memory up/down loads to the GPU. Also you can easily
use multiple OpenCL devices (just change the 'dev' variable in
CLSolver::solve)

/ Mathias Broxvall, August 2014
