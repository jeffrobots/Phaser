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
#define TS .000022675737 //Sample period

// Each element will hold a left and right sample
typedef struct STEREO {
float s_left;
float s_right;
} s_int16;

s_int16 input[BUFSIZE];
s_int16 filtered[BUFSIZE];
volatile int minf, maxf, BW; //GEL Controlled
volatile float lfo_freq, mix;
float t; //Keep track of global time

float amp, offset, c, d, c1, c2;
double fc, width;



AIC31_data_type codec_data;

void APBandReject() {

  //Compute LFO components
  amp = (maxf-minf)/2;
  offset = (maxf+minf)/2;
  fc = amp*sin(2*PI*lfo_freq*t) + offset; // Compute center frequency based on an LFO
  width = BW/FS; //Normalized bandwidth of filter

  // Calculate coefficients
  //d = (-0.9889);
  //c = (-0.8668);
  c = -1*cos(2*PI*fc/FS);
  d = (tan(PI*width)-1)/(tan(PI*width)+1);
  c1 = (1-c)/2;
  c2 = (1-c)*d;


  //  Calculate new outputs using difference equations
  filtered[0].s_right = c1*input[0].s_right + c2*input[1].s_right
         + input[2].s_right*c1 - c2*filtered[1].s_right
         + c*filtered[2].s_right;

  filtered[0].s_left = c1*input[0].s_left + c2*input[1].s_left
         + input[2].s_left*c1 - c2*filtered[1].s_left
         + c*filtered[2].s_left;

  return;
}

interrupt void interrupt4(void) // interrupt service routine
{
  int i;
  t = (t<511) ? t+=TS : 0; //Reset time before it overflows
  //t = (t + TS) % 511;
  codec_data.uint = input_sample();
  // left and right channel inputs
  input[0].s_left = (float) codec_data.channel[LEFT];
  input[0].s_right =(float) codec_data.channel[RIGHT];

  // Run filtering
  APBandReject();

  //output stereo signal
  codec_data.channel[LEFT] = (uint16_t) (filtered[0].s_left*mix + input[0].s_left*(1-mix));
  codec_data.channel[RIGHT] = (uint16_t) (filtered[0].s_right*mix + input[0].s_right*(1-mix));
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
  t = 0;
  minf = 200;
  maxf = 10000; // These will change with GEL files
  mix = .8;
  lfo_freq = 1; //Fill with good starting values
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
