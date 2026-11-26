/*
    PluginEditor.h

    Three knobs (Input Gain, Ceiling, Release) and a gain-reduction meter
    driven by a Timer polling the processor.
 
    + an OUTPUT true-peak(dBTP) meter with peak hold.
    + click the dBTP meter to reset the hold value.
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
    void mouseDown (const juce::MouseEvent& e) override;

private:
    void timerCallback() override;
    void setupSlider (juce::Slider& slider, juce::Label& label, const juce::String& name);
    
    juce::Rectangle<int> getMeterColumnBounds() const;
    juce::Rectangle<int> getGrMeterBounds() const;
    juce::Rectangle<int> getTruePeakMeterBounds() const;

    LimiterAudioProcessor& processor;

    juce::Slider inputGainSlider, ceilingSlider, releaseSlider;
    juce::Label inputGainLabel, ceilingLabel, releaseLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> inputGainAttachment, ceilingAttachment, releaseAttachment;

    // Display-only state
    float displayedGrDb = 0.0f;
    float truePeakBarDb = -100.0f;   // falling bar
    float truePeakHoldDb = -100.0f;  // highest value since the last reset
 
    static constexpr float tpMeterMinDb = -30.0f;
    static constexpr float tpMeterMaxDb = 3.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LimiterAudioProcessorEditor)
};
