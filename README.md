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

expected speed
==============

On an AMD 7970 you can run 1024 variables in 0.6 seconds, 8192
variable in 39 seconds.  

Note that the execution speed appears to scale quadratically instead
of cubically - this has to do with the additional number of cores of
the GPU that can be utilized. To see a cubic scaling phenomena you
need to use atleast 4x the number of cores as your number of variables
due to how the work groups are split up on the GPU.  

/ Mathias Broxvall, August 2014
