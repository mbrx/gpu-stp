CC	= g++
# Linux
MATHFLAGS = -ftree-vectorize -ffast-math -fassociative-math -freciprocal-math -fno-signed-zeros -msse2 -msse -msse3  -O3
CFLAGS = -I. -I/usr/X11R6/include -I/sw/include -c ${MATHFLAGS} -DLINUX -g -O3 -I/opt/AMDAPP/include -std=c99 -g
LDFLAGS = -L/usr/X11R6/lib -L/sw/lib -lm -O3 -g -lOpenCL

OBJS = main.o opencl.o clsolver.o

all: main

clean: 
	rm *.o *~

main: ${OBJS}
	${CC} ${OBJS} -o main ${LDFLAGS}

%.o: %.cc
	${CC} -c $< -o $@ ${CFLAGS}


