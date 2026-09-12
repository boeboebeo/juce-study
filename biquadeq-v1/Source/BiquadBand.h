/*
 BiquadBand.h
 
 Design notes:
 - Each BiquadBand instance owns BOTH the coefficients AND the per-channel
   state (x1, x2, y1, y2). This means a stereo signal needs TWO BiquadBand
   instances per filter stage (one per channel) so their histories don't mix.
   See PluginProcessor.h/.cpp for how the instances are allocated per channel.
 - Coefficients are stored already normalized (a0 = 1), same as the Python
   code's final normalization step.
 
 */
