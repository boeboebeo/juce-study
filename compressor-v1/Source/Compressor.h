/*
 Compressor.h
 
 - Feedforward dynamic compressor
    + switchable Peak / RMS detection
    + soft-knee static curve
    + branching attack/release ballistics smoothing
    (applied in the dB domain, on the gain-reduction signal,
    not on the raw input level)
    + stereo Linked detection
 
 
 - Giannoulis, Massberg, Reiss, "Digital Dynamic Range Compressor Design --
 A Tutorial and Analysis" (JAES, 2012) -- 이 논문에서의 표준 feedforward compressor topology 를 따름
 */
