#ifndef __SO_AND_CHEN__
#define __SO_AND_CHEN__

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "define.h"

static const uint32_t enhanced_point = SAMPLING_RATE * 0.35f;

#define signal_window_size 5
static SignalPoint signal_window[signal_window_size];
static uint32_t signal_window_count = 0;

static SignalPoint slop;

static bool so_chen_init_flag = false;
static uint32_t init_count = 0;

static bool maxi_init = false;
static float maxi;

static float slop_threshold = 0;

static SignalPoint qrs_onset_point;

static int qrs_on_set_count = 0;
static int qrs_onset_idx = 0;
static bool qrs_on_set_flag = false;  

static SignalPoint max;
static SignalPoint max_slop;
static bool max_init = false;

static float last_maxi = 0;
static uint32_t last_point = 0;

SignalPoint So_Chen_detect(SignalPoint,int,float,float);

#endif
