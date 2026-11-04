/*
    PluginEditor.h

    Three knobs (Input Gain, Ceiling, Release) and a gain-reduction meter
    driven by a Timer polling the processor.
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class LimiterAudioProcessorEditor : public juce::AudioProcessorEditor,
                                     private juce::Timer
{
public:
    explicit LimiterAudioProcessorEditor (LimiterAudioProcessor& p);
    ~LimiterAudioProcessorEditor() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;
    void setupSlider (juce::Slider& slider, juce::Label& label, const juce::String& name);

    LimiterAudioProcessor& processor;

    juce::Slider inputGainSlider, ceilingSlider, releaseSlider;
    juce::Label inputGainLabel, ceilingLabel, releaseLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> inputGainAttachment, ceilingAttachment, releaseAttachment;

    // Smoothed for display only.
    float displayedGrDb = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LimiterAudioProcessorEditor)
};
