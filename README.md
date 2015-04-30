# Phaser
LFO Controlled Phaser using C 
By jeffrobots

## phaser.c
*This c file contains the main phaser implementation in C for the TI OMAP LCDK.
*Proper setup of the device can be found in OMAPStartUp.pdf

**Important note**
phaser.c will not compile without all relevent header files. These files need to be taken from a computer that has been properly set up to compile for the device in Code Composer.

## LFO_phaser1.m and AP_band_reject.m
*These two matlab files are demonstration and proof of concept for the C version of the phaser. 
*With string.wav in the working directory of MATLAB, running LFO_phaser1 will yield an output wav file that has been phase shifted using a low frequency oscillator.



