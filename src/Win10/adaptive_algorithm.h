#ifndef __ALGORITHM__
#define __ALGORITHM__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "define.h"

static const uint32_t MEAN_SIZE = SAMPLING_RATE;
static uint32_t mean_count;
static float mean_sum;
static float mean;      

static const uint32_t RMS_SIZE = SAMPLING_RATE;
static uint32_t rms_count;
static float rms_sum;
static float rms;

static const uint32_t CV_SIZE = SAMPLING_RATE;
static uint32_t cv_count;
static float sd;
static float cv;

static float current_max;
static float current_min;
static int max_point;
static int min_point;
static SignalPoint peak;
static bool is_detecting_emi;
static bool init_flag = false;

extern float CalculateMean(float);
extern float CalculateRootMeanSquare(float);
extern float CalculateCoefficientOfVariation(float);
extern void InitPeakDetect(float,bool);
extern SignalPoint PeakDetect(float,int,float,bool*);

#endif
