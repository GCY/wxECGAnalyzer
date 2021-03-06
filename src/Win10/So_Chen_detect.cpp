#include "So_Chen_detect.h"

SignalPoint So_Chen_detect(SignalPoint signal,int initial_point,float threshold_parameter,float filter_parameter)
{
   /* init slop window pool, size = 5 */
   if(signal_window_count < signal_window_size){
      signal_window[signal_window_count%signal_window_size] = signal;
      ++signal_window_count;
      SignalPoint value;
      value.index = -1;
      return value;
   }
   else{
      signal_window[signal_window_count%signal_window_size] = signal;
      ++signal_window_count;
      SignalPoint value;
   }

   /* calculate slop */
   uint32_t idx_for_slop = signal_window_count-2;
   slop.value = ( (-2.0f * signal_window[(idx_for_slop-2)%signal_window_size].value) - signal_window[(idx_for_slop-1)%signal_window_size].value + signal_window[(idx_for_slop+1)%signal_window_size].value + (2.0f * signal_window[(idx_for_slop+2)%signal_window_size].value) );
   slop.index = signal_window[idx_for_slop%signal_window_size].index;

   /* init maxi */
   if(!so_chen_init_flag){
      if(!maxi_init){
	 max.value = 0;
	 max.index = -1;	 
	 maxi = slop.value;
	 maxi_init = true;
      }      
      ++init_count;
      if(init_count > initial_point){
	 so_chen_init_flag = true;
	 /* calculate slop threshold */
	 slop_threshold = threshold_parameter / 16.0f * maxi;
      }
      if(slop.value > maxi){
	 maxi = slop.value;
      }     
      SignalPoint value;
      value.index = -1;
      return value;           
   }

   /* detect QRS complex on set */
   if(qrs_on_set_flag && (signal_window_count - last_point > enhanced_point)){
      if(!max_init){
	 max = signal_window[(idx_for_slop)%signal_window_size];
	 max_init = true;
      }
      if(signal_window[(idx_for_slop)%signal_window_size].value > max.value){
	 max = signal_window[(idx_for_slop)%signal_window_size];
	 max_slop = slop;
      }
      else if(signal_window[(idx_for_slop)%signal_window_size].value < max.value){
	 last_point = signal_window_count;
	 qrs_on_set_flag = false;
	 max_init = false;
#ifdef _WIN_
	 maxi = (( fabsf(max.value - qrs_onset_point.value) - maxi) / filter_parameter) + maxi;
#else
	 maxi = ((fabs(max.value - qrs_onset_point.value) - maxi) / filter_parameter) + maxi;
#endif
	 slop_threshold = threshold_parameter / 16.0f * maxi;
	 last_maxi = maxi;
	 return max;
      }
   }
   else{
      if(slop.value > slop_threshold){
	 ++qrs_on_set_count;
      }
      else if(qrs_on_set_count){
	 qrs_on_set_count = 0;
      }

      if(qrs_on_set_count >= 2){ // is QRS complex on set
	 qrs_on_set_flag = true;
	 qrs_on_set_count = 0;
	 qrs_onset_idx = idx_for_slop;
	 qrs_onset_point = signal;
      }
      else if((signal_window_count - last_point > enhanced_point * 2) && (slop_threshold > 0)){ //decay threshold

	 slop_threshold -= slop.value;

	 if((signal_window_count - last_point > SAMPLING_RATE * 3)){ //threshold oscillating
	    slop_threshold -= ((signal_window_count - last_point) >> (int)threshold_parameter);
	 }
      }
   }

   SignalPoint value;
   value.index = -1;
   return value;
}
