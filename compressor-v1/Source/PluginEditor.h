/*
    PluginEditor.h

    Knobs for threshold/ratio/knee/attack/release/rmsWindow/makeup, a combo
    box to pick Peak vs RMS detection, and a simple gain-reduction meter
    driven by a Timer polling the processor.
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class CompressorAudioProcessorEditor : public juce::AudioProcessorEditor,
                                        private juce::Timer
{
public:
    explicit CompressorAudioProcessorEditor (CompressorAudioProcessor& p);
    ~CompressorAudioProcessorEditor() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;
    void setupSlider (juce::Slider& slider, juce::Label& label, const juce::String& name);

    CompressorAudioProcessor& processor;

    juce::Slider thresholdSlider, ratioSlider, kneeSlider;
    juce::Slider attackSlider, releaseSlider, rmsWindowSlider, makeupSlider;
    juce::ComboBox detectorBox;

    juce::Label thresholdLabel, ratioLabel, kneeLabel;
    juce::Label attackLabel, releaseLabel, rmsWindowLabel, makeupLabel, detectorLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<SliderAttachment>
        thresholdAttachment, ratioAttachment, kneeAttachment,
        attackAttachment, releaseAttachment, rmsWindowAttachment, makeupAttachment;
    std::unique_ptr<ComboAttachment> detectorAttachment;

    // Smoothed for display only, so the meter doesn't flicker sample-to-sample.
    float displayedGrDb = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CompressorAudioProcessorEditor)
};
