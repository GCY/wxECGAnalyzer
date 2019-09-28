#include "FIR.h"

float FIR_filter(float input)
{
   
   float *coeff     = coefficients;
   float *coeff_end = coefficients + taps;

   float *buf_val = buffer + offset;

   *buf_val = input;
   float output_ = 0;

   while(buf_val >= buffer){
      output_ += *buf_val-- * *coeff++;
   }

   buf_val = buffer + taps-1;

   while(coeff < coeff_end){
      output_ += *buf_val-- * *coeff++;
   }

   if(++offset >= taps){
      offset = 0;
   }

   return output_;
}

void FIR_reset_buffer()
{
   memset(buffer, 0, sizeof(float) * taps);
   offset = 0;
}
