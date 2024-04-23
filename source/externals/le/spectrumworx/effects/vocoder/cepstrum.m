function cepstrum( input_file, frame_size, cutoff_frequency )

[signal, sample_rate] = wavread( input_file );

cutoff_bin    = uint32( cutoff_frequency / sample_rate * frame_size );
eight_khz_bin = uint32( 6000             / sample_rate * frame_size );

frames          = vec2mat( signal, frame_size )';
frame_index     = uint32( floor( length( frames( 1, : ) ) / 2 ) );
frame_spectrums = abs( fft( frames ) );
frame           = frame_spectrums( :, frame_index );
frame_log       = log( frame );
cepstrum        = real( ifft( frame_log ) );

window_brick = boxcar( length( cepstrum ) );
window_brick( cutoff_bin + 1 : end - cutoff_bin ) = 0;

window_brick_smoothed = window_brick;
window_brick_smoothed( cutoff_bin - 1 ) = 0.75;
window_brick_smoothed( cutoff_bin     ) = 0.50;
window_brick_smoothed( cutoff_bin + 1 ) = 0.25;
window_brick_smoothed( end - ( cutoff_bin - 1 ) ) = 0.75;
window_brick_smoothed( end - ( cutoff_bin     ) ) = 0.50;
window_brick_smoothed( end - ( cutoff_bin + 1 ) ) = 0.25;

window_brick_udo = window_brick * 2;
window_brick_udo( cutoff_bin + 1 : end ) = 0;
window_brick_udo( 1          ) = 1;
window_brick_udo( cutoff_bin ) = 1;

window_hamming_aux = hamming( double( cutoff_bin * 2 ), 'periodic' );
window_hamming = [ window_hamming_aux( cutoff_bin + 1 : end ); zeros( frame_size - 2 * cutoff_bin, 1 ); window_hamming_aux( 1 : cutoff_bin ) ];

cepstrum_brick = cepstrum .* window_brick;
envelope_brick = real( fft( cepstrum_brick ) );

cepstrum_brick_smoothed = cepstrum .* window_brick_smoothed;
envelope_brick_smoothed = real( fft( cepstrum_brick_smoothed ) );

cepstrum_brick_udo = cepstrum .* window_brick_udo;
envelope_brick_udo = real( fft( cepstrum_brick_udo ) );

cepstrum_hamming = cepstrum .* window_hamming;
envelope_hamming = real( fft( cepstrum_hamming ) );

x_axis = 0 : eight_khz_bin - 1;
x_axis = x_axis * ( sample_rate / frame_size );
figure( 1 );
plot( frames( :, frame_index ) );
figure( 2 );
hold off;
plot( x_axis, frame_log              ( 1 : eight_khz_bin, : ), 'b' );
hold on;
plot( x_axis, envelope_brick         ( 1 : eight_khz_bin, : ), 'r' );
plot( x_axis, envelope_hamming       ( 1 : eight_khz_bin, : ), 'g' );
plot( x_axis, envelope_brick_smoothed( 1 : eight_khz_bin, : ), 'k' );
plot( x_axis, envelope_brick_udo     ( 1 : eight_khz_bin, : ), 'c' );

end
