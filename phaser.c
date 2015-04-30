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
struct STEREO {
int16_t s_left;
int16_t s_right;
};



typedef struct STEREO s_int16;

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
  filtered[0].s_left = (1-c)/2*input[0].s_left - (1-c)*d*input[1].s_left
         + input[2].s_left*(1-c)/2 - d*(1-c)*filtered[1].s_left
         + c*filtered[2].s_left;

  filtered[0].s_right = (1-c)/2*input[0].s_right - (1-c)*d*input[1].s_right
         + input[2].s_right*(1-c)/2 - d*(1-c)*filtered[1].s_right
         + c*filtered[2].s_right;

  return;
}

interrupt void interrupt4(void) // interrupt service routine
{
  int i;
  codec_data.uint = input_sample();
  // left and right channel inputs
  input[0].s_left = codec_data.channel[LEFT];
  input[0].s_right = codec_data.channel[RIGHT];

  // Run filtering
  APBandReject();

  //output stereo signal
  codec_data.channel[LEFT] = filtered[0].s_left*mix + input[0].s_left*(1-mix);
  codec_data.channel[RIGHT] = filtered[0].s_right*mix + input[0].s_right*(1-mix);
  output_sample(codec_data.uint);

  // Need to shift arrays to prepare for next sample
  for(i=BUFSIZE-1; i>0; i--){
    input[i].s_left = input[i-1].s_left;
    input[i].s_right = input[i-1].s_right;
    filtered[i].s_left = filtered[i-1].s_left;
    filtered[i].s_right = filtered[i-1].s_right;
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
    input[i].s_left = 0;
    input[i].s_right = 0;
    filtered[i].s_left = 0;
    filtered[i].s_right = 0;
  }
  L138_initialise_intr(FS_44100_HZ,ADC_GAIN_0DB,DAC_ATTEN_0DB,LCDK_LINE_INPUT);
  while(1);
}
