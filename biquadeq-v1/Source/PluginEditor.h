/*
    PluginEditor.h

    Minimal GUI: 7 rotary knobs (HP Freq/Q, Peak Freq/Q/Gain, LP Freq/Q),
    each bound to the matching APVTS parameter through a SliderAttachment.
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class ThreeBandEQAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit ThreeBandEQAudioProcessorEditor (ThreeBandEQAudioProcessor& p);
    ~ThreeBandEQAudioProcessorEditor() override = default;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    void setupSlider (juce::Slider& slider, juce::Label& label, const juce::String& name);

    void layoutBand (juce::Rectangle<int> column,
                      std::initializer_list<juce::Slider*> sliders,
                      std::initializer_list<juce::Label*> labels);

    ThreeBandEQAudioProcessor& processor;

    juce::Slider hpFreqSlider, hpQSlider;
    juce::Slider peakFreqSlider, peakQSlider, peakGainSlider;
    juce::Slider lpFreqSlider, lpQSlider;

    juce::Label hpFreqLabel, hpQLabel;
    juce::Label peakFreqLabel, peakQLabel, peakGainLabel;
    juce::Label lpFreqLabel, lpQLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
        hpFreqAttachment, hpQAttachment,
        peakFreqAttachment, peakQAttachment, peakGainAttachment,
        lpFreqAttachment, lpQAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThreeBandEQAudioProcessorEditor)
};
