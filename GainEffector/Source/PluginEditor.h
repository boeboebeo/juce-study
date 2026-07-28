#pragma once

#include "PluginProcessor.h"

class GainEffectorAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit GainEffectorAudioProcessorEditor (GainEffectorAudioProcessor&);
    ~GainEffectorAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    GainEffectorAudioProcessor& audioProcessor;

    juce::Slider gainSlider;
    juce::Label  gainLabel;
    juce::ToggleButton phaseInvertButton;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<SliderAttachment> gainAttachment;
    std::unique_ptr<ButtonAttachment> phaseInvertAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GainEffectorAudioProcessorEditor)
};
