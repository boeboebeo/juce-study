/*
    PluginEditor.cpp
*/

#include "PluginEditor.h"

LimiterAudioProcessorEditor::LimiterAudioProcessorEditor (LimiterAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setupSlider (inputGainSlider, inputGainLabel, "Input Gain");
    setupSlider (ceilingSlider,   ceilingLabel,   "Ceiling");
    setupSlider (releaseSlider,   releaseLabel,   "Release");

    auto& state = processor.apvts;

    inputGainAttachment = std::make_unique<SliderAttachment> (state, "inputGain", inputGainSlider);
    ceilingAttachment   = std::make_unique<SliderAttachment> (state, "ceiling",   ceilingSlider);
    releaseAttachment   = std::make_unique<SliderAttachment> (state, "release",   releaseSlider);

    setSize (460, 260);
    startTimerHz (30);
}

LimiterAudioProcessorEditor::~LimiterAudioProcessorEditor()
{
    stopTimer();
}

void LimiterAudioProcessorEditor::setupSlider (juce::Slider& slider, juce::Label& label,
                                                const juce::String& name)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 18);
    addAndMakeVisible (slider);

    label.setText (name, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (label);
}

void LimiterAudioProcessorEditor::timerCallback()
{
    // Display-only smoothing so the meter is readable; it does not touch audio.
    const float targetDb = processor.getCurrentGainReductionDb();
    displayedGrDb += 0.3f * (targetDb - displayedGrDb);
    repaint();
}

void LimiterAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (18.0f);
    g.drawFittedText ("Limiter", getLocalBounds().removeFromTop (28),
                       juce::Justification::centred, 1);

    // Gain reduction meter on the right: empty = 0 dB, full = 18 dB or more.
    auto meterArea = getLocalBounds().reduced (10).removeFromRight (36);
    meterArea.removeFromTop (28);

    g.setColour (juce::Colours::black);
    g.fillRect (meterArea);

    const float maxGrDb = 18.0f;
    const float fraction = juce::jlimit (0.0f, 1.0f, -displayedGrDb / maxGrDb);
    auto filled = meterArea.removeFromTop (juce::roundToInt (meterArea.getHeight() * fraction));

    g.setColour (juce::Colours::orange);
    g.fillRect (filled);
}

void LimiterAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (10);
    area.removeFromTop (28);    // title
    area.removeFromRight (46);  // meter column

    const int knobWidth = area.getWidth() / 3;

    auto layoutKnob = [&] (juce::Slider& s, juce::Label& l)
    {
        auto column = area.removeFromLeft (knobWidth);
        l.setBounds (column.removeFromTop (18));
        s.setBounds (column);
    };

    layoutKnob (inputGainSlider, inputGainLabel);
    layoutKnob (ceilingSlider,   ceilingLabel);
    layoutKnob (releaseSlider,   releaseLabel);
}
