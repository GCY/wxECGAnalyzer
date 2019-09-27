#ifndef __HRV__
#define __HRV__

#include <math.h>

#include "define.h"

const uint32_t MAX_BEAT_MS = 135; // 360Hz/(160/60) , hr=160
const uint32_t MIN_BEAT_MS = 480; // 360Hz/(45/60) , hr=45

static const uint32_t BEAT_SIZE = 16;
float beat_array[BEAT_SIZE];
uint32_t beat_count = 0;

static const uint32_t BEAT_DIFF_SIZE = BEAT_SIZE - 1;
float beat_diff_array[BEAT_DIFF_SIZE];	// unit ms

void AddBeatArray(float index)
{/*
   if(beat_count < BEAT_SIZE){
      beat_array[beat_count] = index;
   }
   else{
      beat_array[beat_count%BEAT_SIZE] = index;
   }*/
   beat_array[beat_count%BEAT_SIZE] = index;
   ++beat_count;
}

float HRV_SDNN()
{
   if(beat_count >= BEAT_SIZE){
      for(int i = 0;i < BEAT_DIFF_SIZE;++i){
	 beat_diff_array[i] = (beat_array[(beat_count+1+i)%BEAT_SIZE] - beat_array[(beat_count+i)%BEAT_SIZE]) * (1000.0f/(float)SAMPLING_RATE);
	 //beat_diff_array[i] = (beat_array[(beat_count+i)%BEAT_SIZE] - beat_array[((beat_count-1)+i)%BEAT_SIZE]) * (1.0f/1000.0f);
      }

      float sum = 0;
      for(int i = 0;i < BEAT_DIFF_SIZE;++i){
	 sum += beat_diff_array[i];
      }
      float mean = sum / BEAT_DIFF_SIZE;
      sum = 0;
      for(int i = 0;i < BEAT_DIFF_SIZE;++i){
	 sum += (beat_diff_array[i]-mean) * (beat_diff_array[i]-mean);
      }
      float sd = sqrt(sum/BEAT_DIFF_SIZE);
      //return sd;
      return sd;

   }
   else{
      return 0;
   }
}

float PulseRate()
{
   if(beat_count >= BEAT_SIZE){   
      float sum = 0;
      uint32_t count = 0;
      for(int i = 0;i < BEAT_DIFF_SIZE;++i){
	 sum += 1000.0f / beat_diff_array[i] * 60.0f;
      }
      return sum / BEAT_DIFF_SIZE;
   }
   return 0;
}

int NN50()
{
   if(beat_count >= BEAT_SIZE){
      uint32_t nn50 = 0;
      for(int i = 1;i < BEAT_DIFF_SIZE-1;++i){
	 if((beat_diff_array[(beat_count-1+i)%BEAT_DIFF_SIZE] - beat_diff_array[(beat_count-2+i)%BEAT_DIFF_SIZE]) > 50){
	    ++nn50;
	 }
      }
      return nn50;
   }
   return -1;
}

#endif
