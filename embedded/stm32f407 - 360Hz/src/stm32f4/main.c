#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stm32f4xx.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_usart.h>
#include <stm32f4xx_exti.h>
#include <stm32f4xx_i2c.h>
#include <stm32f4xx_adc.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_dma.h>
#include <stm32f4xx_rtc.h>

#include "./usb_cdc_device/usbd_usr.h"
#include "./usb_cdc_device/usbd_cdc_core.h"
#include "./usb_cdc_device/usb_conf.h"
#include "./usb_cdc_device/usbd_desc.h"
#include "./usb_cdc_device/usbd_cdc_vcp.h"

#include "tiny_printf.h"

#include "./algorithm/define.h"
#include "./algorithm/FIR.h"
#include "./algorithm/adaptive_algorithm.h"
#include "./algorithm/HC_Chen_detect.h"
#include "./algorithm/So_Chen_detect.h"
#include "./algorithm/Pan_Tompkins_detect.h"

/* ECG signal input is PC0 pin*/

//Enable FPU
//SCB->CPACR |= ((3UL<< 10*2)|(3UL << 11*2));
#define __FPU_PRESENT
#define __FPU_USED

#define VCP_MODE

//#define SINGLE_MODE
//#define ATA
//#define HC
//#define SO
//#define PT

volatile uint32_t TimingDelay;

volatile uint32_t micros = 0;

//uint32_t SAMPLING_RATE = 360;

void Delay(__IO uint32_t nTime)
{
   TimingDelay = nTime;
   while(TimingDelay){
   }
}

void SysTick_Handler(void)
{
   if(TimingDelay){
      --TimingDelay;
   }
   ++micros;
}

void Init_LED()
{
   //Enable the GPIOD Clock
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE);


   // GPIOD Configuration
   GPIO_InitTypeDef GPIO_InitStruct;
   GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
   GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
   GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;

   GPIO_ResetBits(GPIOD, GPIO_Pin_12);
   GPIO_ResetBits(GPIOD, GPIO_Pin_13);
   GPIO_ResetBits(GPIOD, GPIO_Pin_14);
   GPIO_ResetBits(GPIOD, GPIO_Pin_15);
   GPIO_Init(GPIOD, &GPIO_InitStruct);   
}

#define ADC3_DR_ADDRESS (uint32_t)ADC3_BASE+0x4C
volatile uint32_t ADC3_value[4];
volatile bool  ADC3_ready;

void ADC_4_Channel_IO_Init( void )
{
   GPIO_InitTypeDef GPIO_InitStructure;

   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;;//ADC0~3
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
   GPIO_Init( GPIOC, &GPIO_InitStructure );
}

void ADC_4_Channel_DMA_Init( void )
{
   DMA_InitTypeDef DMA_InitStructure;
   NVIC_InitTypeDef   NVIC_InitStructure;

   DMA_InitStructure.DMA_Channel = DMA_Channel_2;
   DMA_InitStructure.DMA_PeripheralBaseAddr = ( uint32_t )ADC3_DR_ADDRESS;
   DMA_InitStructure.DMA_Memory0BaseAddr = ( uint32_t ) &ADC3_value;
   DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
   DMA_InitStructure.DMA_BufferSize = 4; 
   DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
   DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
   DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
   DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_Word;
   DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
   DMA_InitStructure.DMA_Priority = DMA_Priority_High;
   DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
   DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
   DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
   DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

   DMA_Init( DMA2_Stream0, &DMA_InitStructure );

   NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream0_IRQn;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);

   DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, ENABLE);

   DMA_Cmd( DMA2_Stream0, ENABLE );
}

void ADC_4_Channel_AD_Init( void )
{
   ADC_InitTypeDef ADC_InitStructure;
   ADC_CommonInitTypeDef ADC_CommonInitStructure;

   ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
   ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;
   ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
   ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_SampleTime_3Cycles;
   ADC_CommonInit( &ADC_CommonInitStructure );

   ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
   ADC_InitStructure.ADC_ScanConvMode = ENABLE;
   ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
   ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;
   ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
   ADC_InitStructure.ADC_NbrOfConversion = 4;
   ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T4_CC4;

   ADC_Init( ADC3, &ADC_InitStructure );

   ADC_RegularChannelConfig( ADC3, ADC_Channel_10, 1, ADC_SampleTime_15Cycles );
   ADC_RegularChannelConfig( ADC3, ADC_Channel_11, 2, ADC_SampleTime_15Cycles );
   ADC_RegularChannelConfig( ADC3, ADC_Channel_12, 3, ADC_SampleTime_15Cycles );
   ADC_RegularChannelConfig( ADC3, ADC_Channel_13, 4, ADC_SampleTime_15Cycles );

   ADC_DMARequestAfterLastTransferCmd( ADC3, ENABLE );
   ADC_DMACmd( ADC3, ENABLE );
   ADC_Cmd( ADC3, ENABLE );
}

void TIM4_init( void )
{
   TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
   TIM_OCInitTypeDef  TIM_OCInitStructure;


   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

   // Timer Freq = 84000000 / (1000000/SAMPLING_RATE) / 84(Prescaler)
   TIM_TimeBaseStructure.TIM_Period = (1000000/SAMPLING_RATE)-1;
   TIM_TimeBaseStructure.TIM_Prescaler = (84-1);
   TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
   TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

   TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

   // PWM Dyty Cycle = 50 / (1000000/SAMPLING_RATE)
   TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
   TIM_OCInitStructure.TIM_Pulse = 50;
   TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;

   TIM_OC4Init(TIM4, &TIM_OCInitStructure);

   TIM_Cmd( TIM4, ENABLE );


}

void ADC_4_Channel_ReInit( void )
{
   RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_DMA2 | RCC_AHB1Periph_GPIOC, ENABLE );
   RCC_APB2PeriphClockCmd( RCC_APB2Periph_ADC3, ENABLE );

   ADC_4_Channel_IO_Init();

   DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, DISABLE);
   DMA_Cmd( DMA2_Stream0, DISABLE );	

   ADC_4_Channel_DMA_Init();	
   ADC_4_Channel_AD_Init();

   TIM4_init();

}

void ADC_4_Channel_Init( void )
{
   RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_DMA2 | RCC_AHB1Periph_GPIOC, ENABLE );
   RCC_APB2PeriphClockCmd( RCC_APB2Periph_ADC3, ENABLE );
   ADC_4_Channel_IO_Init();
   ADC_4_Channel_DMA_Init();
   ADC_4_Channel_AD_Init();
   TIM4_init();
}

void DMA2_Stream0_IRQHandler(void)
{

   DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TCIF0);

   ADC3_ready	= true;	
}

volatile uint32_t adc_value[4] = {0};
void Init_ADC(void)
{
   GPIO_InitTypeDef gpio;
   DMA_InitTypeDef dma;
   NVIC_InitTypeDef nvic;
   ADC_InitTypeDef adc;
   ADC_CommonInitTypeDef adc_com;

   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 | RCC_AHB1Periph_GPIOC,ENABLE);
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3,ENABLE);

   dma.DMA_Channel = DMA_Channel_2;
   dma.DMA_PeripheralBaseAddr = (uint32_t)0x4001224C;
   dma.DMA_Memory0BaseAddr = (uint32_t)&adc_value;
   dma.DMA_DIR = DMA_DIR_PeripheralToMemory;
   dma.DMA_BufferSize = 4;
   dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
   dma.DMA_MemoryInc = DMA_MemoryInc_Enable;
   dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
   dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
   dma.DMA_Mode = DMA_Mode_Circular;
   dma.DMA_Priority = DMA_Priority_High;
   dma.DMA_FIFOMode = DMA_FIFOMode_Disable;
   dma.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
   dma.DMA_MemoryBurst = DMA_MemoryBurst_Single;
   dma.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
   DMA_Init(DMA2_Stream0, &dma);
   DMA_Cmd(DMA2_Stream0, ENABLE);

   gpio.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
   gpio.GPIO_Mode = GPIO_Mode_AN;
   gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
   GPIO_Init(GPIOC,&gpio);

   adc_com.ADC_Mode = ADC_Mode_Independent;
   adc_com.ADC_Prescaler = ADC_Prescaler_Div4;
   adc_com.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
   adc_com.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
   ADC_CommonInit(&adc_com);

   adc.ADC_Resolution = ADC_Resolution_12b;
   adc.ADC_ScanConvMode = ENABLE;
   adc.ADC_ContinuousConvMode = ENABLE;
   adc.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
   adc.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
   adc.ADC_DataAlign = ADC_DataAlign_Right;
   adc.ADC_NbrOfConversion = 4;
   ADC_Init(ADC3,&adc);

   ADC_RegularChannelConfig(ADC3,ADC_Channel_10,1,ADC_SampleTime_3Cycles);
   ADC_RegularChannelConfig(ADC3,ADC_Channel_11,2,ADC_SampleTime_3Cycles);
   ADC_RegularChannelConfig(ADC3,ADC_Channel_12,3,ADC_SampleTime_3Cycles);
   ADC_RegularChannelConfig(ADC3,ADC_Channel_13,4,ADC_SampleTime_3Cycles);
   ADC_DMARequestAfterLastTransferCmd(ADC3,ENABLE);
   ADC_Cmd(ADC3,ENABLE);
   ADC_SoftwareStartConv(ADC3);
   ADC_DMACmd(ADC3,ENABLE);


}

/* Private */
#define USB_VCP_RECEIVE_BUFFER_LENGTH		128
uint8_t INT_USB_VCP_ReceiveBuffer[USB_VCP_RECEIVE_BUFFER_LENGTH];
uint32_t int_usb_vcp_buf_in, int_usb_vcp_buf_out, int_usb_vcp_buf_num;
USB_VCP_Result USB_VCP_INT_Status;
//extern LINE_CODING linecoding;
uint8_t USB_VCP_INT_Init = 0;
USB_OTG_CORE_HANDLE	USB_OTG_dev;

extern uint8_t INT_USB_VCP_ReceiveBuffer[USB_VCP_RECEIVE_BUFFER_LENGTH];

USB_VCP_Result USBVCPInit(void)
{
   USBD_Init(&USB_OTG_dev,
#ifdef USE_USB_OTG_FS
	 USB_OTG_FS_CORE_ID,
#else
	 USB_OTG_HS_CORE_ID,
#endif
	 &USR_desc, 
	 &USBD_CDC_cb, 
	 &USR_cb);   

   /* Reset buffer counters */
   int_usb_vcp_buf_in = 0;
   int_usb_vcp_buf_out = 0;
   int_usb_vcp_buf_num = 0;

   /* Initialized */
   USB_VCP_INT_Init = 1;

   return USB_VCP_OK;
}

USB_VCP_Result USB_VCP_GetStatus(void) {
   if (USB_VCP_INT_Init) {
      return USB_VCP_INT_Status;
   }
   return USB_VCP_ERROR;
}

USB_VCP_Result USB_VCP_Getc(uint8_t* c) {
   /* Any data in buffer */
   if (int_usb_vcp_buf_num > 0) {
      /* Check overflow */
      if (int_usb_vcp_buf_out >= USB_VCP_RECEIVE_BUFFER_LENGTH) {
	 int_usb_vcp_buf_out = 0;
      }
      *c = INT_USB_VCP_ReceiveBuffer[int_usb_vcp_buf_out];
      INT_USB_VCP_ReceiveBuffer[int_usb_vcp_buf_out] = 0;

      /* Set counters */
      int_usb_vcp_buf_out++;
      int_usb_vcp_buf_num--;

      /* Data OK */
      return USB_VCP_DATA_OK;
   }
   *c = 0;
   /* Data not ready */
   return USB_VCP_DATA_EMPTY;
}

USB_VCP_Result USB_VCP_Putc(volatile char c) {
   uint8_t ce = (uint8_t)c;

   /* Send data over USB */
   VCP_DataTx(&ce, 1);

   /* Return OK */
   return USB_VCP_OK;
}

USB_VCP_Result USB_VCP_Puts(char* str) {
   while (*str) {
      USB_VCP_Putc(*str++);
   }

   /* Return OK */
   return USB_VCP_OK;
}

USB_VCP_Result INT_USB_VCP_AddReceived(uint8_t c) {
   /* Still available data in buffer */
   if (int_usb_vcp_buf_num < USB_VCP_RECEIVE_BUFFER_LENGTH) {
      /* Check for overflow */
      if (int_usb_vcp_buf_in >= USB_VCP_RECEIVE_BUFFER_LENGTH) {
	 int_usb_vcp_buf_in = 0;
      }
      /* Add character to buffer */
      INT_USB_VCP_ReceiveBuffer[int_usb_vcp_buf_in] = c;
      /* Increase counters */
      int_usb_vcp_buf_in++;
      int_usb_vcp_buf_num++;

      /* Return OK */
      return USB_VCP_OK;
   }

   /* Return Buffer full */
   return USB_VCP_RECEIVE_BUFFER_FULL;
}

int main(void)
{
   if(SysTick_Config(SystemCoreClock / 1000 / 1000)){
      while(1){}
   }

   USBVCPInit();
   Delay(1000);
   //Init_ADC(); //ADC3 Pooling
   ADC_4_Channel_Init(); // DMA + TIMER + FIFO, 360Hz sampling rate
   Delay(1000);

   Init_LED();

   uint32_t start_time = micros;
   uint32_t time_to_index = 0;
   uint32_t last_qrs_point = 0;
   uint32_t brightness_time = micros;

   int id_state = 0;
   int recheck_state = 0;
   int sn = 1;  

   while(1){
#ifdef SINGLE_MODE
      if(micros - brightness_time > 70000){
	GPIO_ResetBits(GPIOD, GPIO_Pin_12);
	GPIO_ResetBits(GPIOD, GPIO_Pin_13);
	GPIO_ResetBits(GPIOD, GPIO_Pin_14);
	GPIO_ResetBits(GPIOD, GPIO_Pin_15);	 
	}

      if(ADC3_ready){
	 ADC3_ready = false;
	 ++time_to_index;

	 //start_time = micros;
	 float adc3_ch0 = ADC3_value[0]; 
	 float value = FIR_filter(adc3_ch0);
	 start_time = micros;
#ifdef ATA
	 static const float CV_LIMIT = 50.0f;
	 static const float THRESHOLD_FACTOR = 3.0f;
	 double mean = CalculateMean(value);
	 double rms = CalculateRootMeanSquare(value);
	 double cv = CalculateCoefficientOfVariation(value);
	 double threshold;
	 if(cv > CV_LIMIT){
	    threshold = rms;
	 }
	 else{
	    threshold = rms * (cv/100.0f) * THRESHOLD_FACTOR;
	 }		  
	 bool is_peak;
	 SignalPoint result;
	 result = PeakDetect(value,time_to_index,threshold,&is_peak);
	 if(result.index != -1){
	    if(is_peak){	 
	       last_qrs_point = time_to_index;
	       brightness_time = micros;
	       GPIO_SetBits(GPIOD, GPIO_Pin_12);
	       GPIO_SetBits(GPIOD, GPIO_Pin_13);
	       GPIO_SetBits(GPIOD, GPIO_Pin_14);
	       GPIO_SetBits(GPIOD, GPIO_Pin_15);			     
	    }
	 }
#endif

#ifdef HC
	 float result = value;
	 bool is_peak = HC_Chen_detect(result);
	 if(is_peak){
	    last_qrs_point = time_to_index;
	    brightness_time = micros;
	    GPIO_SetBits(GPIOD, GPIO_Pin_12);
	    GPIO_SetBits(GPIOD, GPIO_Pin_13);
	    GPIO_SetBits(GPIOD, GPIO_Pin_14);
	    GPIO_SetBits(GPIOD, GPIO_Pin_15);				  
	 }
#endif

#ifdef SO
	 float result = value;
	 SignalPoint sp;
	 sp.value = result;
	 sp.index = time_to_index;
	 SignalPoint peak = So_Chen_detect(sp,SAMPLING_RATE * 0.25f,4,16);
	 if(peak.index != -1){
	    last_qrs_point = time_to_index;
	    brightness_time = micros;
	    GPIO_SetBits(GPIOD, GPIO_Pin_12);
	    GPIO_SetBits(GPIOD, GPIO_Pin_13);
	    GPIO_SetBits(GPIOD, GPIO_Pin_14);
	    GPIO_SetBits(GPIOD, GPIO_Pin_15);		      
	 }

#endif

#ifdef PT

	 double result = value;
	 double bandpass;
	 double integral;
	 double square;

	 bandpass = result;
	 result = Derivative(result);
	 result = Squar(result);
	 square = result;
	 result = MovingWindowIntegral(result);
	 integral = result;

	 SignalPoint peak = ThresholdCalculate(time_to_index,value,bandpass,square,integral);

	 if(peak.index != -1){
	    last_qrs_point = time_to_index;
	    brightness_time = micros;
	    GPIO_SetBits(GPIOD, GPIO_Pin_12);
	    GPIO_SetBits(GPIOD, GPIO_Pin_13);
	    GPIO_SetBits(GPIOD, GPIO_Pin_14);
	    GPIO_SetBits(GPIOD, GPIO_Pin_15);	      
	 }

#endif
	 if(USB_VCP_GetStatus() == USB_VCP_CONNECTED) {
	    char diff_time_str[255];
	    sprintf(diff_time_str,"%d\n",micros - start_time);
	    USB_VCP_Puts(diff_time_str);     
	 }
      }
#endif

#ifdef VCP_MODE  
      if(USB_VCP_GetStatus() == USB_VCP_CONNECTED) {
	 if(sn == 1){
	    uint8_t c;
	    if (USB_VCP_Getc(&c) == USB_VCP_DATA_OK) {
	       //TM_USB_VCP_Putc(c);
	       if(c == 'A' && id_state == 0){
		  ++id_state;
	       }
	       if(c == 'C' && id_state == 1){
		  ++id_state;
	       }
	       if(c == 'K' && id_state == 2){
		  ++id_state;
		  USB_VCP_Puts("ACK");
		  sn = 0;

	       }
	    }
	 }
	 else{
	    uint8_t c;
	    if (USB_VCP_Getc(&c) == USB_VCP_DATA_OK) {
	       //TM_USB_VCP_Putc(c);
	       if(c == 'G' && recheck_state == 0){
		  ++recheck_state;
	       }
	       if(c == 'O' && recheck_state == 1){
		  ++recheck_state;
	       }
	       if(c == 'T' && recheck_state == 2){
		  ++recheck_state;
		  USB_VCP_Puts("GOT");
	       }
	    }
	 }

	 if(recheck_state == 3){
	    if(ADC3_ready){
	       ADC3_ready = false;
	       unsigned char str[255];
	       sprintf(str,"R%d,%d,%d,%d,%d,",micros, ADC3_value[0], ADC3_value[1], ADC3_value[2], ADC3_value[3]); 
	       USB_VCP_Puts(str);

	    }
	 }
      }
#endif  
      Delay(1);
   }


   return 0;
}    
