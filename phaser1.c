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
#define BW 1000

// Each element will hold a left and right sample
typedef struct STEREO {
float s_left;
float s_right;
} s_int16;


//int16_t Linput,Rinput;
s_int16 input[BUFSIZE];
s_int16 filtered[BUFSIZE];
int minf, maxf; //GEL Controlled
float lfo_freq, mix;
float t; //Keep track of global time



AIC31_data_type codec_data;

void APBandReject() {
  float amp, offset, c, d, c1, c2;
  double fc, width;
  //Compute LFO components
  t = t+1/FS; // New sample, update time
  //amp = (maxf-minf)/2;
  //offset = (maxf+minf)/2;
  //fc = amp*sin(2*PI*lfo_freq*t) + offset; // Compute center frequency based on an LFO
  width = BW/FS; //Normalized bandwidth of filter
  fc = 1000;
  // Calculate coefficients
  d = (-0.9889);
  c = (-0.8668);
  c1 = (1-c)/2;
  c2 = (1-c)*d;


  //  Calculate new outputs using difference equations
  //  May need to typecast
 /* filtered[0].s_left = .9334*input[0].s_left - 1.84608*input[1].s_left
         + input[2].s_left*.9334 - 1.84608*filtered[1].s_left
         - .9889*filtered[2].s_left;*/


  filtered[0].s_right = c1*input[0].s_right + c2*input[1].s_right
         + input[2].s_right*c1 - c2*filtered[1].s_right
         + c*filtered[2].s_right;

  /*filtered[0].s_right = .9334*input[0].s_right + (-1.84608)*input[1].s_right
         + input[2].s_right*.9334 - (-1.84608)*filtered[1].s_right
         + (-.8668)*filtered[2].s_right;*/

 //Lowpass
 /*filtered[0].s_left = 1.7991*filtered[1].s_left - .8175*filtered[2].s_left + .0046*input[0].s_left
		  	  	  	  + .0092*input[1].s_left + .0046*input[2].s_left;*/


  return;
}

interrupt void interrupt4(void) // interrupt service routine
{
  int i;
  codec_data.uint = input_sample();
  // left and right channel inputs
  input[0].s_left = (float) codec_data.channel[LEFT];
  input[0].s_right =(float) codec_data.channel[RIGHT];

  // Run filtering
  APBandReject();

  //output stereo signal
  codec_data.channel[LEFT] = (int16_t) (filtered[0].s_right*mix + input[0].s_right*(1-mix));
  codec_data.channel[RIGHT] = (int16_t) (filtered[0].s_right*mix + input[0].s_right*(1-mix));
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
  mix = 1;
  lfo_freq = 1;
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
