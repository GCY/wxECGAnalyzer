#ifndef __FIR_H__
#define __FIR_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/*360hz 0.51Hz~8.9Hz 20190925*/
#define taps 32
static const float coefficients[taps] = {0.012177,0.01599,0.019905,0.02387,0.027827,0.031719,0.035487,0.039075,0.042426,0.045488,0.048212,0.050553,0.052475,0.053944,0.054937,0.055438,0.055438,0.054937,0.053944,0.052475,0.050553,0.048212,0.045488,0.042426,0.039075,0.035487,0.031719,0.027827,0.02387,0.019905,0.01599,0.012177};

static float buffer[taps];
unsigned offset;

extern float FIR_filter(float);
extern void FIR_reset_buffer();

#endif
