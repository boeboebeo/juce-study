/*
    PluginEditor.cpp
*/

#include "PluginEditor.h"

ThreeBandEQAudioProcessorEditor::ThreeBandEQAudioProcessorEditor (ThreeBandEQAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setupSlider (hpFreqSlider,   hpFreqLabel,   "HP Freq");
    setupSlider (hpQSlider,      hpQLabel,      "HP Q");
    setupSlider (peakFreqSlider, peakFreqLabel, "Peak Freq");
    setupSlider (peakQSlider,    peakQLabel,    "Peak Q");
    setupSlider (peakGainSlider, peakGainLabel, "Peak Gain");
    setupSlider (lpFreqSlider,   lpFreqLabel,   "LP Freq");
    setupSlider (lpQSlider,      lpQLabel,      "LP Q");

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    auto& state = processor.apvts;

    hpFreqAttachment   = std::make_unique<Attachment> (state, "hpFreq",   hpFreqSlider);
    hpQAttachment      = std::make_unique<Attachment> (state, "hpQ",      hpQSlider);
    peakFreqAttachment = std::make_unique<Attachment> (state, "peakFreq", peakFreqSlider);
    peakQAttachment    = std::make_unique<Attachment> (state, "peakQ",    peakQSlider);
    peakGainAttachment = std::make_unique<Attachment> (state, "peakGain", peakGainSlider);
    lpFreqAttachment   = std::make_unique<Attachment> (state, "lpFreq",   lpFreqSlider);
    lpQAttachment      = std::make_unique<Attachment> (state, "lpQ",      lpQSlider);

    setSize (600, 260);
}

void ThreeBandEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.setColour (juce::Colours::white);
    g.setFont (18.0f);
    g.drawFittedText ("3-Band EQ (RBJ Biquad)",
                       getLocalBounds().removeFromTop (28),
                       juce::Justification::centred, 1);
}

void ThreeBandEQAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (10);
    area.removeFromTop (28); // title bar

    auto columnWidth = area.getWidth() / 3;

    layoutBand (area.removeFromLeft (columnWidth),
                { &hpFreqSlider, &hpQSlider },
                { &hpFreqLabel, &hpQLabel });

    layoutBand (area.removeFromLeft (columnWidth),
                { &peakFreqSlider, &peakQSlider, &peakGainSlider },
                { &peakFreqLabel, &peakQLabel, &peakGainLabel });

    layoutBand (area,
                { &lpFreqSlider, &lpQSlider },
                { &lpFreqLabel, &lpQLabel });
}

void ThreeBandEQAudioProcessorEditor::setupSlider (juce::Slider& slider, juce::Label& label,
                                                    const juce::String& name)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 18);
    addAndMakeVisible (slider);

    label.setText (name, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (label);
}

void ThreeBandEQAudioProcessorEditor::layoutBand (juce::Rectangle<int> column,
                                                   std::initializer_list<juce::Slider*> sliders,
                                                   std::initializer_list<juce::Label*> labels)
{
    const int n = (int) sliders.size();
    const int rowHeight = column.getHeight() / n;

    auto sliderIt = sliders.begin();
    auto labelIt  = labels.begin();

    for (int i = 0; i < n; ++i)
    {
        auto row = column.removeFromTop (rowHeight);
        (*labelIt)->setBounds (row.removeFromTop (18));
        (*sliderIt)->setBounds (row);
        ++sliderIt;
        ++labelIt;
    }
}
