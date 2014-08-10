/* general.h
   Contains common type definitions, includes, etc. for all labs
*/

/* First, standard includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef LINUX
#ifndef DARWIN
#include <windows.h>
#endif
#endif

/* OpenGL includes */
#include <GL/gl.h>
#include <GL/glu.h>

/* SDL related includes */
#include "SDL/SDL.h"
#include "SDL/SDL_mouse.h"
#include "SDL/SDL_keyboard.h"
#include "SDL/SDL_keysym.h"
#include "SDL/SDL_image.h"


extern double gTime;
extern GLUquadricObj *qobj;

#define MIN(x,y) ((x)<(y)?(x):(y))
#define MAX(x,y) ((x)>(y)?(x):(y))

#ifndef M_PI
#define M_PI 3.14159265
#endif

typedef struct sCarState {
  float carPos[2], carRot, wheelRot, currSpeed;
} CarState;

typedef struct sMove {
  float wheelRot, speed;
} Move;
