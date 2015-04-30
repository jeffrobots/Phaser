clear all 
close all
mix = .75;
	inputf = 'string.wav';
	[input, Fs] = wavread(inputf);
	input = input(:,1); %%Select only one channel

	minf = 200;
	maxf = 10000;
	t = 0:(length(input)-1/Fs);
	t = t/Fs;
	A = (maxf-minf)/2;
	Fph = 1; %% .5Hz LFO
	offset = (maxf+minf)/2;
	%%%% In C
	%		FLFO = .5 //hz

	%%%% t = 0.0
	%%%% Fs = 44100
	%%%% TS = 1/Fs;
	%%%% interrupt{
	% 	t = t+Ts;
	% 	LFO = Amplitude*sin(2*pi*FLFO*t);

	% }
	fc = A*sin(2*pi*Fph*t) + offset;


	width = 5000/Fs; %% 50 hz width

	y = AP_band_reject(input, fc, width, Fs); %%Pass input through allpass phaser
    y = y/max(abs(y));
    output = (y*mix)+(input'*(1-mix));
    wavwrite(output, Fs, 'output.wav');
    
	figure
	plot(t,input, 'r');
    hold on
	plot(t,output, 'b');
    hold on
	title('Phaser (blue) vs Original Wave (red)')



