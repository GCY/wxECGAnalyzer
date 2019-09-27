#ifndef __HC_CHEN__
#define __HC_CHEN__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "define.h"

static const uint32_t M = 9;
static const uint32_t N = SAMPLING_RATE * 0.15f;
static const uint32_t  window_size = SAMPLING_RATE;
static const float HP_CONSTANT = ((float) 1.0f / (float) M);   
// circular buffer for input ecg signal
// we need to keep a history of M + 1 samples for HP filter
static float ecg_buff[M + 1] = {0};
static int ecg_buff_WR_idx = 0;
static int ecg_buff_RD_idx = 0;

// circular buffer for input ecg signal
// we need to keep a history of N+1 samples for LP filter
static float hp_buff[N + 1] = {0};
static int hp_buff_WR_idx = 0;
static int hp_buff_RD_idx = 0;

// LP filter outputs a single point for every input point
// This goes straight to adaptive filtering for eval
static float next_eval_pt = 0;

// running sums for HP and LP filters, values shifted in FILO
static float hp_sum = 0;
static float lp_sum = 0;

// parameters for adaptive thresholding
static float treshold = 0;
static bool triggered = false;
static int trig_time = 0;
static float win_max = 0;
static int win_idx = 0;
static int number_iter = 0;

static int sample = 0;
static int last_qrs_point = 0;

static int DELAY_TIME = window_size * 0.5;

extern bool HC_Chen_detect(float);

#endif
