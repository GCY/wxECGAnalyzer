#ifndef __PAN_TOMPKINS__
#define __PAN_TOMPKINS__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include "define.h"

extern int TwoPoleRecursive(int);

extern int LowPassFilter(int);
extern int HighPassFilter(int);

extern int Derivative(int);
extern int Squar(int);
extern int MovingWindowIntegral(int);

extern SignalPoint ThresholdCalculate(int,float,int,int,int);
#endif
