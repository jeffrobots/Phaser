function LFO_phaser()
	clear all;
	close all;
	inputf = 'string.wav';
	[input, Fs, N] = wavread(inputf);
	minf = 200;
	maxf = 8000;
	Fph = 10;
	deltaF = Fph/Fs % Normalize phaser oscillation to sampling freq
	%%Create lookup table of center frequencies
	
	fc = minf:deltaF:maxf;
	padnum = length(input);
	fc = padarray(fc, [0 padnum], 'symmetric', 'post');
	%% Make sure that the array fc is the proper length
	fc = fc(1:length(input));
	%%%% In C
	%		FLFO = 20 //hz

	%%%% t = 0.0
	%%%% Fs = 44100
	%%%% TS = 1/Fs;
	%%%% interrupt interrupt4{
	% 	t = t+Ts;
	% 	LFO = Amplitude*sin(2*pi*FLFO*t);

	% }
	figure()
	plot(fc)

	width = 300; %% 50 hz width
	BW = 2*width/Fs;

	y = AP_band_reject(input, fc, BW);
	y = y/max(abs(y));
	wavwrite(y, Fs, N, 'output.wav');

	figure()
	hold on
	plot(input, 'r');
	plot(y, 'b');
	title('Phaser (blue) vs Original Wave (red)')
end


function y = AP_band_reject(x, fc, BW)
	%% x is input sample waveform
	%% fc is input center frequency
	%% BW is input bandreject bandwidth

	c = (tan(pi*BW/2)-1) / (tan(pi*BW/2) + 1);
	
	xh = [0, 0];
	for n=1:length(x)
		d = -cos(pi*fc(n));
		xh_new = x(n) - d*(1-c)*xh(1) + c*xh(2);
		ap_y = -c*xh_new + d*(1-c)*xh(1) + xh(2);
		xh = [xh_new, xh(1)];
		y(n) = .5*(x(n)-ap_y); % Bandreject
	end
end
