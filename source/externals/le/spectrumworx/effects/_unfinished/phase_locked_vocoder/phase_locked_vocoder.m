%% Phase locked vocoder testing. 
%  See the paper "New phase vocoder techniques for pitch shifting,
%  harmonizing and other exotic effetcs" by Dolson.

overlap = 4;
n = 1024;
zpad = 1;
npad = n * zpad;
hop = n/overlap;
w = hanning(n + 1);
w = w(1:n);

scale = 1.6;
[x, fs] = wavread('c:/le/media/chaotica.wav');
x = x(1:200000, 1); % extract only one channel
y = zeros(size(x));

nframes = floor(length(x)/n * overlap);

k = (0:(npad/2))';

ptr = 1;
frameidx = 0;

maxpeaks = 100;
peakbin = zeros(100, 1);

summph = 0;
oldph = zeros(n/2+1, 1);

while (ptr < length(x) - n)
    fprintf('Frame %d/%d\n', frameidx, nframes);
    frameidx = frameidx + 1;
    inframe = x(ptr:(ptr+n-1)) .* w;
    frameft = fft(inframe);
    frameft = frameft(1:(n/2+1));
    
    mag = abs(frameft);
    ph = angle(frameft);
    
    % Peak detection stage.
    
    % Sort the magnitudes.
    
    [mags, idxs] = sort(mag, 'descend');
    
    % Identifty the maxpeaks largest peaks using a very simple strategy.
    npeaks = 0;
    peakidx = 0;
    while (npeaks < maxpeaks) && (peakidx < n/2 + 1)
        peakidx = peakidx + 1;
        
        if idxs(peakidx) == 1 || idxs(peakidx) == n/2 + 1
            continue;
        end
        
        if (mag(idxs(peakidx)-1) < mag(idxs(peakidx))) && ...
           (mag(idxs(peakidx)+1) < mag(idxs(peakidx)))
            npeaks = npeaks + 1;
            peakbin(npeaks) = idxs(peakidx);
        end
    end
    
    % let's traverse the peaks from left to right
    peakbin(1:npeaks) = sort(peakbin(1:npeaks), 'ascend');
    
    outmag = zeros(n, 1);
    outphase = zeros(n, 1);
    for peakidx = 1:npeaks
        % find the beggining and the end of the region of influence for the
        % peakidx-th peak
        
        if peakidx > 1
            bin1 = floor( (peakbin(peakidx) + peakbin(peakidx-1)) / 2);
        else
            bin1 = 1;
        end
        if peakidx < npeaks
            bin2 = ceil( (peakbin(peakidx) + peakbin(peakidx+1)) / 2 );
        else
            bin2 = n/2 + 1;
        end

        bin1 = max(bin1, 1);
        bin2 = min(bin2, n/2+1);
        
        if bin1 == bin2
            fprintf('Sranje\n');
        end
        
        % Translate the peaks to new locations. At this moment ignore the
        % spillovers (cut them off)
        
        newpeakbin = (peakbin(peakidx) - 1) * scale + 1;
        deltapeakbin = newpeakbin - peakbin(peakidx);
        
        outmag( (bin1:bin2) + floor(deltapeakbin) ) ...
            = interp1((bin1:bin2)+deltapeakbin, ...
              mag(bin1:bin2), ...
              (bin1:bin2)+floor(deltapeakbin), ...
              'spline', 'extrap', 'spline');
        
        if any(isnan(outmag))
            fprintf('Sranje\n')
        end
          
        % phase shift for the whole region (phase coherence)
        dw  = (scale - 1) * (peakbin(peakidx) - 1) / n * 2 * pi;
        dph = dw * 1 / fs * n / overlap;
        
        outphase( (bin1:bin2) + floor(deltapeakbin) ) ...
            = oldph(bin1:bin2) + ph(bin1:bin2) + dph * frameidx;
    end
    oldph = outphase;
    
    % synthesize

    outft = outmag .* exp(1i*outphase);
    outft = outft(1:(n/2+1));

    outft = [outft; conj(outft((end-1):(-1):2))];
    outft(n/2+1) = real(outft(n/2+1));
    outft(1) = real(outft(1));
    outframe = ifft(outft);
    
    y(ptr:(ptr+n-1)) = y(ptr:(ptr+n-1)) + outframe(1:n) .* w;
    ptr = ptr + hop; 
end


