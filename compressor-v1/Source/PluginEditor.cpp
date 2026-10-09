/*
    PluginEditor.cpp
*/

#include "PluginEditor.h"

CompressorAudioProcessorEditor::CompressorAudioProcessorEditor (CompressorAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setupSlider (thresholdSlider, thresholdLabel, "Threshold");
    setupSlider (ratioSlider,     ratioLabel,     "Ratio");
    setupSlider (kneeSlider,      kneeLabel,      "Knee");
    setupSlider (attackSlider,    attackLabel,    "Attack");
    setupSlider (releaseSlider,   releaseLabel,   "Release");
    setupSlider (rmsWindowSlider, rmsWindowLabel, "RMS Win");
    setupSlider (makeupSlider,    makeupLabel,    "Makeup");

    detectorBox.addItem ("Peak", 1);
    detectorBox.addItem ("RMS", 2);
    addAndMakeVisible (detectorBox);

    detectorLabel.setText ("Detector", juce::dontSendNotification);
    detectorLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (detectorLabel);

    auto& state = processor.apvts;

    thresholdAttachment = std::make_unique<SliderAttachment> (state, "threshold", thresholdSlider);
    ratioAttachment     = std::make_unique<SliderAttachment> (state, "ratio",     ratioSlider);
    kneeAttachment      = std::make_unique<SliderAttachment> (state, "knee",      kneeSlider);
    attackAttachment    = std::make_unique<SliderAttachment> (state, "attack",    attackSlider);
    releaseAttachment   = std::make_unique<SliderAttachment> (state, "release",   releaseSlider);
    rmsWindowAttachment = std::make_unique<SliderAttachment> (state, "rmsWindow", rmsWindowSlider);
    makeupAttachment    = std::make_unique<SliderAttachment> (state, "makeup",    makeupSlider);
    detectorAttachment  = std::make_unique<ComboAttachment>  (state, "detector",  detectorBox);

    setSize (720, 320);
    startTimerHz (30); // meter refresh rate
}

CompressorAudioProcessorEditor::~CompressorAudioProcessorEditor()
{
    stopTimer();
}

void CompressorAudioProcessorEditor::setupSlider (juce::Slider& slider, juce::Label& label,
                                                   const juce::String& name)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 18);
    addAndMakeVisible (slider);

    label.setText (name, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (label);
}

void CompressorAudioProcessorEditor::timerCallback()
{
    // Simple exponential smoothing purely for a readable meter -- this has
    // nothing to do with the compressor's own attack/release ballistics,
    // which already happened on the audio thread.
    const float targetDb = processor.getCurrentGainReductionDb();
    displayedGrDb += 0.3f * (targetDb - displayedGrDb);
    repaint();
}

void CompressorAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (18.0f);
    g.drawFittedText ("Compressor", getLocalBounds().removeFromTop (28),
                       juce::Justification::centred, 1);

    // Gain reduction meter: a vertical bar in the top-right corner.
    // 0 dB GR = empty, -24 dB GR (or more) = full.
    auto meterArea = getLocalBounds().removeFromTop (140).removeFromRight (40).reduced (4);
    g.setColour (juce::Colours::black);
    g.fillRect (meterArea);

    const float maxGrDb = 24.0f;
    const float fraction = juce::jlimit (0.0f, 1.0f, -displayedGrDb / maxGrDb);
    auto filled = meterArea.removeFromBottom (juce::roundToInt (meterArea.getHeight() * fraction));

    g.setColour (juce::Colours::orange);
    g.fillRect (filled);
}

void CompressorAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (10);
    area.removeFromTop (28); // title
    area.removeFromTop (140); // meter zone, laid out only in paint()

    const int numKnobs = 7;
    const int knobWidth = area.getWidth() / numKnobs;

    auto layoutKnob = [&] (juce::Slider& s, juce::Label& l)
    {
        auto column = area.removeFromLeft (knobWidth);
        l.setBounds (column.removeFromTop (18));
        s.setBounds (column);
    };

    layoutKnob (thresholdSlider, thresholdLabel);
    layoutKnob (ratioSlider,     ratioLabel);
    layoutKnob (kneeSlider,      kneeLabel);
    layoutKnob (attackSlider,    attackLabel);
    layoutKnob (releaseSlider,   releaseLabel);
    layoutKnob (rmsWindowSlider, rmsWindowLabel);
    layoutKnob (makeupSlider,    makeupLabel);

    // Detector combo box under the title, left side.
    auto comboArea = getLocalBounds().reduced (10).removeFromTop (28 + 140).removeFromBottom (24);
    detectorLabel.setBounds (comboArea.removeFromLeft (70));
    detectorBox.setBounds (comboArea.removeFromLeft (100));
}
