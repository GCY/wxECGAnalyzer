#include "Pan_Tompkins_detect.h"

/* y(nT) = 1.875y(nT – T) – 0.9219y(nT – 2T) + x (nT) – x(nT – 2T) */
int TwoPoleRecursive(int data)
{
   static int xnt, xm1, xm2, ynt, ym1, ym2 = 0;

   xnt = data;
   ynt = (ym1 + (ym1 >> 1) + (ym1 >> 2) + (ym1 >> 3)) +					// 1.875 = 1 + 1/2 + 1/4 + 1/8
      (((ym2 >> 1) + (ym2 >> 2) + (ym2 >> 3) + (ym2 >> 5) + (ym2 >> 6)) + xnt - xm2);	// 0.916 = 1 + 1/2 + 1/4 + 1/8 + 1/32 + 1/64
   xm2 = xm1;
   xm1 = xnt;
   xm2 = ym1;
   ym2 = ym1;
   ym1 = ynt;
   return ynt;
}

/* y(nT) = 2y(nT – T) – y(nT – 2T) + x(nT) – 2x(nT – 6T) + x(nT – 12T) */
int LowPassFilter(int data)
{
   static int y1 = 0, y2 = 0, x[26], n = 12;
   int y0;

   x[n] = x[n + 13] = data;
   y0 = (y1 << 1) - y2 + x[n] - (x[n + 6] << 1) + x[n + 12];
   y2 = y1;
   y1 = y0;
   y0 >>= 5;
   if(--n < 0){
      n = 12;
   }
   return y0;
}

/* p(nT) = x(nT – 16T) – 32 [y(nT – T) + x(nT) – x(nT – 32T)] */
int HighPassFilter(int data)
{
   static int y1 = 0, x[66], n = 32;
   int y0;

   x[n] = x[n + 33] = data;
   y0 = y1 + x[n] - x[n + 32];
   y1 = y0;
   if(--n < 0){
      n = 32;
   }
   return (x[n + 16] - (y0 >> 5));
}

/* y = 1/8 (2x( nT) + x( nT - T) - x( nT - 3T) - 2x( nT - 4T)) */
int Derivative(int data)
{
   int y;
   static int x_derv[4];

   y = (data << 1) + x_derv[3] - x_derv[1] - ( x_derv[0] << 1);
   y >>= 3;
   for(int i = 0; i < 3; ++i){
      x_derv[i] = x_derv[i + 1];
   }
   x_derv[3] = data;
   return y;
}

int Squar(int data)
{
   return (data * data);
}

/* y(nT) = 1/N [x(nT – (N – 1)T) + x(nT – (N – 2)T) +...+ x(nT)] */
int MovingWindowIntegral(int data)
{
   //static const int WINDOW_SIZE = SAMPLING_RATE * 0.2;
   #define WINDOW_SIZE 72

   static int x[WINDOW_SIZE], ptr = 0;
   static long sum = 0;
   long ly;
   int y;

   if(++ptr == WINDOW_SIZE){
      ptr = 0;
   }
   sum -= x[ptr];
   sum += data;
   x[ptr] = data;
   ly = sum >> 5;
   uint32_t MAX_INTEGRAL = 4096;//32400;
   if(ly > MAX_INTEGRAL){
      y = MAX_INTEGRAL;
   }
   else{
      y = (int)ly;
   }
   return (y);
}

SignalPoint ThresholdCalculate(int sample,float value,int bandpass,int square,int integral)
{
   //static const int QRS_TIME = SAMPLING_RATE * 0.1;
   //static const int SEARCH_BACK_TIME = SAMPLING_RATE * 1.66f;
   #define QRS_TIME 36
   #define SEARCH_BACK_TIME 598

   static int bandpass_buffer[SEARCH_BACK_TIME],integral_buffer[SEARCH_BACK_TIME];
   static SignalPoint peak_buffer[SEARCH_BACK_TIME];
   static int square_buffer[QRS_TIME];
   static long unsigned last_qrs = 0, last_slope = 0, current_slope = 0;
   static int peak_i = 0, peak_f = 0, threshold_i1 = 0, threshold_i2 = 0, threshold_f1 = 0, threshold_f2 = 0, spk_i = 0, spk_f = 0, npk_i = 0, npk_f = 0;
   static bool qrs, regular = true, prev_regular;
   static int rr1[8]={0}, rr2[8]={0}, rravg1, rravg2, rrlow = 0, rrhigh = 0, rrmiss = 0;

   SignalPoint result;
   result.index = -1;

   peak_buffer[sample%SEARCH_BACK_TIME].index = sample;
   peak_buffer[sample%SEARCH_BACK_TIME].value = value;
   bandpass_buffer[sample%SEARCH_BACK_TIME] = bandpass;
   integral_buffer[sample%SEARCH_BACK_TIME] = integral;
   square_buffer[sample%QRS_TIME] = square;

   // If the current signal is above one of the thresholds (integral or filtered signal), it's a peak candidate.
   if(integral >= threshold_i1 || bandpass >= threshold_f1){
      peak_i = integral;
      peak_f = bandpass;
   }

   // If both the integral and the signal are above their thresholds, they're probably signal peaks.
   if((integral >= threshold_i1) && (bandpass >= threshold_f1)){
      // There's a 200ms latency. If the new peak respects this condition, we can keep testing.
      if(sample > last_qrs + SAMPLING_RATE*0.2f){
	 //if(sample > last_qrs + (SAMPLING_RATE*0.2f)){
	 // If it respects the 200ms latency, but it doesn't respect the 360ms latency, we check the slope.
	 if(sample <= last_qrs + (long unsigned int)(0.36*SAMPLING_RATE)){
	    // The squared slope is "M" shaped. So we have to check nearby samples to make sure we're really looking
	    // at its peak value, rather than a low one.
	    int current = sample;
	    current_slope = 0;
	    for(int j = current - QRS_TIME; j <= current; ++j){
	       if(square_buffer[j%QRS_TIME] > current_slope){
		  current_slope = square_buffer[j%QRS_TIME];
	       }
	    }
	    //current_slope = square;

	    if(current_slope <= (int)(last_slope/2)){
	       qrs = false;
	       //return qrs;
	    }

	    else{
	       spk_i = 0.125*peak_i + 0.875*spk_i;
	       threshold_i1 = npk_i + 0.25*(spk_i - npk_i);
	       threshold_i2 = 0.5*threshold_i1;

	       spk_f = 0.125*peak_f + 0.875*spk_f;
	       threshold_f1 = npk_f + 0.25*(spk_f - npk_f);
	       threshold_f2 = 0.5*threshold_f1;

	       last_slope = current_slope;
	       qrs = true;

	       result.value = value;
	       result.index = sample;
	    }
	 }
	 // If it was above both thresholds and respects both latency periods, it certainly is a R peak.
	 else{
	    int current = sample;
	    current_slope = 0;
	    for(int j = current - QRS_TIME; j <= current; ++j){
	       if(square_buffer[j%QRS_TIME] > current_slope){
		  current_slope = square_buffer[j%QRS_TIME];
	       }
	    }
	    //current_slope = square;

	    spk_i = 0.125*peak_i + 0.875*spk_i;
	    threshold_i1 = npk_i + 0.25*(spk_i - npk_i);
	    threshold_i2 = 0.5*threshold_i1;

	    spk_f = 0.125*peak_f + 0.875*spk_f;
	    threshold_f1 = npk_f + 0.25*(spk_f - npk_f);
	    threshold_f2 = 0.5*threshold_f1;

	    last_slope = current_slope;
	    qrs = true;

	    result.value = value;
	    result.index = sample;	    
	 }
      }
      // If the new peak doesn't respect the 200ms latency, it's noise. Update thresholds and move on to the next sample.
      else{
	 peak_i = integral;
	 npk_i = 0.125*peak_i + 0.875*npk_i;
	 threshold_i1 = npk_i + 0.25*(spk_i - npk_i);
	 threshold_i2 = 0.5*threshold_i1;
	 peak_f = bandpass;
	 npk_f = 0.125*peak_f + 0.875*npk_f;
	 threshold_f1 = npk_f + 0.25*(spk_f - npk_f);
	 threshold_f2 = 0.5*threshold_f1;
	 qrs = false;
	 /*outputSignal[current] = qrs;
	   if (sample > DELAY + BUFFSIZE)
	   output(outputSignal[0]);
	   continue;*/

	 //return qrs;
	 return result;
      }	 
      }

      // If a QRS complex was detected, the RR-averages must be updated.
      if(qrs){
	 // Add the newest RR-interval to the buffer and get the new average.
	 rravg1 = 0;
	 for (int i = 0; i < 7; ++i){
	    rr1[i] = rr1[i+1];
	    rravg1 += rr1[i];
	 }
	 rr1[7] = sample - last_qrs;
	 last_qrs = sample;
	 rravg1 += rr1[7];
	 rravg1 *= 0.125;

	 // If the newly-discovered RR-average is normal, add it to the "normal" buffer and get the new "normal" average.
	 // Update the "normal" beat parameters.
	 if ( (rr1[7] >= rrlow) && (rr1[7] <= rrhigh) ){
	    rravg2 = 0;
	    for (int i = 0; i < 7; ++i){
	       rr2[i] = rr2[i+1];
	       rravg2 += rr2[i];
	    }
	    rr2[7] = rr1[7];
	    rravg2 += rr2[7];
	    rravg2 *= 0.125;
	    rrlow = 0.92*rravg2;
	    rrhigh = 1.16*rravg2;
	    rrmiss = 1.66*rravg2;
	 }

	 prev_regular = regular;
	 if(rravg1 == rravg2){
	    regular = true;
	 }
	 // If the beat had been normal but turned odd, change the thresholds.
	 else{
	    regular = false;
	    if (prev_regular){
	       threshold_i1 /= 2;
	       threshold_f1 /= 2;
	    }
	 }      
      }
      // If no R-peak was detected, it's important to check how long it's been since the last detection.
      else{
	 int current = sample;
	 // If no R-peak was detected for too long, use the lighter thresholds and do a back search.
	 // However, the back search must respect the 200ms limit and the 360ms one (check the slope).
	 if((sample - last_qrs > (long unsigned int)rrmiss) && (sample > last_qrs + SAMPLING_RATE*0.2f)){
	    //if((sample - last_qrs > (long unsigned int)rrmiss) && (sample > last_qrs + (SAMPLING_RATE*0.2f))){

	    // If over SEARCH_BACK_TIME of QRS complex
	    if((sample - last_qrs) > SEARCH_BACK_TIME){
	       last_qrs = sample;
	       //return result;
	    }

	    int qrs_last_index = 0; // Last point of QRS complex

	    for(int i = current - (sample - last_qrs) + SAMPLING_RATE*0.2f; i < (long unsigned int)current; ++i){
	       //for(int i = current - (sample - last_qrs) + (SAMPLING_RATE*0.2f); i < (long unsigned int)current; ++i){
	       if((integral_buffer[i%SEARCH_BACK_TIME] > threshold_i2) && (bandpass_buffer[i%SEARCH_BACK_TIME] > threshold_f2)){
		  current_slope = 0;
		  for(int j = current - QRS_TIME; j <= current; ++j){
		     if(square_buffer[j%QRS_TIME] > current_slope){
			current_slope = square_buffer[j%QRS_TIME];
		     }
		  }
		  //current_slope = square;

		  if((current_slope < (int)(last_slope/2)) && (i + sample) < last_qrs + 0.36*last_qrs){
		     qrs = false;
		  }
		  else if(i - last_qrs > 550){
		     peak_i = integral_buffer[i%SEARCH_BACK_TIME];
		     peak_f = bandpass_buffer[i%SEARCH_BACK_TIME];
		     spk_i = 0.25*peak_i+ 0.75*spk_i;
		     spk_f = 0.25*peak_f + 0.75*spk_f;
		     threshold_i1 = npk_i + 0.25*(spk_i - npk_i);
		     threshold_i2 = 0.5*threshold_i1;
		     last_slope = current_slope;
		     threshold_f1 = npk_f + 0.25*(spk_f - npk_f);
		     threshold_f2 = 0.5*threshold_f1;
		     // If a signal peak was detected on the back search, the RR attributes must be updated.
		     // This is the same thing done when a peak is detected on the first try.
		     //RR Average 1
		     rravg1 = 0;
		     for(int j = 0; j < 7; ++j){
			rr1[j] = rr1[j+1];
			rravg1 += rr1[j];
		     }
		     rr1[7] = sample - (current - i) - last_qrs;
		     qrs = true;
		     qrs_last_index = i;
		     last_qrs = sample - (current - i);
		     rravg1 += rr1[7];
		     rravg1 *= 0.125;

		     //RR Average 2
		     if((rr1[7] >= rrlow) && (rr1[7] <= rrhigh)){
			rravg2 = 0;
			for (int i = 0; i < 7; ++i){
			   rr2[i] = rr2[i+1];
			   rravg2 += rr2[i];
			}
			rr2[7] = rr1[7];
			rravg2 += rr2[7];
			rravg2 *= 0.125;
			rrlow = 0.92*rravg2;
			rrhigh = 1.16*rravg2;
			rrmiss = 1.66*rravg2;
		     }

		     prev_regular = regular;
		     if(rravg1 == rravg2){
			regular = true;
		     }
		     else{
			regular = false;
			if(prev_regular){
			   threshold_i1 /= 2;
			   threshold_f1 /= 2;
			}
		     }

		     break;
		  }
	       }
	    }

	    if(qrs){
	       //outputSignal[current] = false;
	       //outputSignal[i] = true;
	       //if (sample > DELAY + BUFFSIZE)
	       //output(outputSignal[0]);
	       //continue;

	       //return qrs;
	       return peak_buffer[qrs_last_index%SEARCH_BACK_TIME];
	    }
	    }

	    // Definitely no signal peak was detected.
	    if(!qrs){
	       // If some kind of peak had been detected, then it's certainly a noise peak. Thresholds must be updated accordinly.
	       if((integral >= threshold_i1) || (bandpass >= threshold_f1)){
		  peak_i = integral;
		  npk_i = 0.125*peak_i + 0.875*npk_i;
		  threshold_i1 = npk_i + 0.25*(spk_i - npk_i);
		  threshold_i2 = 0.5*threshold_i1;
		  peak_f = bandpass;
		  npk_f = 0.125*peak_f + 0.875*npk_f;
		  threshold_f1 = npk_f + 0.25*(spk_f - npk_f);
		  threshold_f2 = 0.5*threshold_f1;
	       }
	    }      
	 }

	 return result;
	 }
