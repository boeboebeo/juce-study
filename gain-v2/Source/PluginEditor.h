#pragma once

#include "PluginProcessor.h"

class gainv2AudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit gainv2AudioProcessorEditor (gainv2AudioProcessor&);
    ~gainv2AudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    gainv2AudioProcessor& audioProcessor;

    juce::Slider gainSlider;
    juce::Label  gainLabel;
    juce::ToggleButton phaseInvertButton;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<SliderAttachment> gainAttachment;
    std::unique_ptr<ButtonAttachment> phaseInvertAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (gainv2AudioProcessorEditor)
};

