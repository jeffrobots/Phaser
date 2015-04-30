function y = AP_band_reject(x, fc, BW, Fs)
	%% x is input sample waveform
	%% fc is input center frequency
	%% BW is input bandreject bandwidth normalized to Fs
	%% %%%%% Divide fc by Fs before you get to this point, remove from input.
	c = (tan(pi*BW)-1) / (tan(pi*BW) + 1);
	
	x1 = 0;
	x2 = 0;
	y1 = 0;
	y2 = 0;
	y = zeros(1, length(x));
	for n=1:length(x)
		d = -cos(2*pi*fc(n)/Fs);
%% 		y(n) = .5*(x(n)*(1-c) + x1*2*d*(1-c) + x2*(1-c)) - y1*d*(1-c) + y2*c;
		y(n) = (1-c)/2*x(n) - (1-c)*d*x1 + x2*(1-c)/2 - d*(1-c)*y1 + c*y2;
		y2 = y1;
		x2 = x1;
		x1 = x(n);
		y1 = y(n);
	end
end