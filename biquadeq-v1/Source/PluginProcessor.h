/*
    PluginProcessor.h
 
    3-band EQ plugin (HighPass -> Peaking -> LowPass), same signal chain as
    the Python `eq = EQ(); eq.add_band(band1..3)` example.
 
    Uses juce::AudioProcessorValueTreeState (APVTS) for parameters, which is
    the standard, DAW-automation-safe way to expose plugin parameters in JUCE.
*/

#pragma once
 
#include <JuceHeader.h>
#include "BiquadBand.h"
