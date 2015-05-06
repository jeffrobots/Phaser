clear all 
close all
mix = .8;
	inputf = 'string.wav';
	[input, Fs] = wavread(inputf);
	input = input(:,1); %%Select only one channel

	minf = 200;
	maxf = 10000;
	t = 0:(length(input)-1/Fs);
	t = t/Fs;
	A = (maxf-minf)/2;
	Fph = .2; %% .5Hz LFO
	offset = (maxf+minf)/2;
	fc = A*sin(2*pi*Fph*t) + offset;
	width = 5000/Fs; %% 50 hz width

	%%Pass input through allpass phaser
	y = AP_band_reject(input, fc, width, Fs); 
    y = y/max(abs(y));
    output = (y'*mix)+(input*(1-mix));
    wavwrite(output, Fs, 'output.wav');
    
	figure
	subplot(3, 1, 1)
	plot(t,input, 'r');

	title('Plot of Original Signal')
	subplot(3,1,2)
	plot(t,output, 'b');
	title('Plot of Signal After Phaser')
	subplot(3, 1, 3)
	%%plot(t, sin(2*3.14*Fph*t), 'g')
	plot(t, fc, 'g')
	title('Plot of LFO Controlled Center Frequencies')





