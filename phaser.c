/*
 * phaser.c
 * LFO Controlled Phaser for TI OMAP LCDK
 *
 *  Created on: APR 29, 2015
 *      Author: jbaatz
 */



#include "L138_LCDK_aic3106_init.h"
#include "math.h"
#define PI 3.141592654
#define FS 44100 //Sampling frequency
#define BUFSIZE 3 //Need 3 samples for difference eqn
#define BW 5000

// Each element will hold a left and right sample
typedef struct STEREO {
  int16_t LEFT;
  int16_t RIGHT;
} s_int16;

//int16_t Linput,Rinput;
s_int16 input[3];
s_int16 filtered[3];
// May not need to be volatile, but it probably should be
// to keep gel files working (Are GELS treated as interupts?)
volatile int minf, maxf, mix; //GEL Controlled
volatile float lfo_freq;
float t; //Keep track of global time



AIC31_data_type codec_data;

void APBandReject() {
  float amp, offset, width, d, c, fc;

  //Compute LFO components
  t = t+1/FS; // New sample, update time
  amp = (maxf-minf)/2;
  offset = (maxf+minf)/2;
  fc = amp*sin(2*PI*lfo_freq*t) + offset; // Compute center frequency based on an LFO
  width = BW/FS; //Normalized bandwidth of filter

  // Calculate coefficients
  d = -cos(2*PI*fc/FS);
  c = (tan(PI*width)-1)/(tan(PI*width)+1);

  //  Calculate new outputs using difference equations
  //  May need to typecast
  filtered[0].LEFT = (1-c)/2*input[0].LEFT - (1-c)*d*input[1].LEFT
         + input[2].LEFT*(1-c)/2 - d*(1-c)*filtered[1].LEFT
         + c*filtered[2].LEFT;
  
  filtered[0].RIGHT = (1-c)/2*input[0].RIGHT - (1-c)*d*input[1].RIGHT
         + input[2].RIGHT*(1-c)/2 - d*(1-c)*filtered[1].RIGHT
         + c*filtered[2].RIGHT;

  return;
}

interrupt void interrupt4(void) // interrupt service routine
{
  int i;
  codec_data.uint = input_sample();
  // left and right channel inputs
  input[0].LEFT = codec_data.channel[LEFT];
  input[0].RIGHT = codec_data.channel[RIGHT];

  // Run filtering
  APBandReject();

  //output stereo signal
  codec_data.channel[LEFT] = filtered[0].LEFT*mix + input[0].LEFT*(1-mix);
  codec_data.channel[RIGHT] = filtered[0].RIGHT*mix + input[0].RIGHT*(1-mix);
  output_sample(codec_data.uint);

  // Need to shift arrays to prepare for next sample
  for(i=BUFSIZE-1; i>0; i--){
    input[i].LEFT = input[i-1].LEFT;
    input[i].RIGHT = input[i-1].RIGHT;
    filtered[i].LEFT = filtered[i-1].LEFT;
    filtered[i].RIGHT = filtered[i-1].RIGHT;
    // 0 indexed samples will be replaced at next interrupt
  }
  return;
}

int main(void)
{
  int i;
  minf = 200; 
  maxf = 10000; // These will change with GEL files
  mix = .5;
  lfo_freq = .5;
  //Initialize buffers
  for(i=BUFSIZE-1; i>=0; i--){
    input[i].LEFT = 0;
    input[i].RIGHT = 0;
    filtered[i].LEFT = 0;
    filtered[i].RIGHT = 0;
  }
  L138_initialise_intr(FS_44100_HZ,ADC_GAIN_0DB,DAC_ATTEN_0DB,LCDK_LINE_INPUT);
  while(1);

  return 0;
}
