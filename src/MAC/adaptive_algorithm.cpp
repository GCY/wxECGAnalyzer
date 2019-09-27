#include "adaptive_algorithm.h"

float CalculateMean(float value)
{
   value /= 1000.0f;
   if(mean_count < MEAN_SIZE){
      mean_sum += value;
      ++mean_count;
   }
   else{
      mean = mean_sum/MEAN_SIZE;
      mean_count = 0;
      mean_sum = 0;

   }
   return (mean * 1000.0f);   
}

float CalculateRootMeanSquare(float value)
{
   value /= 1000.0f;
   if(rms_count < RMS_SIZE){
      rms_sum += value * value;
      ++rms_count;
   }
   else{
      rms = sqrt(rms_sum/RMS_SIZE);
      rms_count = 0;
      rms_sum = 0;

   }
   return (rms * 1000.0f);   
}

float CalculateCoefficientOfVariation(float value)
{
   value /= 1000.0f;
   if(cv_count < CV_SIZE){
      sd += (value - mean)  * (value - mean);
      ++cv_count;
   }
   else{
      sd = sqrt(sd / (CV_SIZE-1));
      cv = (sd / mean) * 100;
      cv_count = 0;
      sd = 0;

   }
   return cv;   
}

void InitPeakDetect(float value,bool emi_first)
{
   if(!init_flag){
      current_max = value;
      current_min = value;
      is_detecting_emi = emi_first;
      init_flag = true;
   }   
}

SignalPoint PeakDetect(float value,int index,float gradient,bool *is_peak)
{
   if(value > current_max){
      max_point = index;
      current_max = value;
   }
   if(value < current_min){
      min_point = index;
      current_min = value;
   }

   if(is_detecting_emi && value < (current_max - gradient) ){

      is_detecting_emi = false;

      current_min = current_max;
      min_point = max_point;
      *is_peak = true;
      peak.value = current_max;
      peak.index = max_point;
      return peak;
   }
   else if((!is_detecting_emi) && value > (current_min + gradient))
   {

      is_detecting_emi = true;

      current_max = current_min;
      max_point = min_point;
      *is_peak = false;
      peak.value = current_min;
      peak.index = min_point;
      return peak;
   }

   peak.index = -1;
   return peak;
}

