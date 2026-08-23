/*
    PluginEditor.cpp
*/

#include "PluginEditor.h"

DelayAudioProcessorEditor::DelayAudioProcessorEditor(DelayAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // ---- Delay Time 슬라이더 ----
    delayTimeSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    delayTimeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(delayTimeSlider);
    addAndMakeVisible(delayTimeLabel);
    delayTimeLabel.setJustificationType(juce::Justification::centred);

    // apvts의 "delayTime" 파라미터와 이 슬라이더를 연결
    // -> 생성 즉시 슬라이더가 현재 파라미터 값으로 초기화되고,
    //    이후 슬라이더를 움직이면 파라미터가 자동으로 갱신됨
    delayTimeAttachment = std::make_unique<SliderAttachment>(
        audioProcessor.apvts, "delayTime", delayTimeSlider);

    // ---- Feedback 슬라이더 ----
    feedbackSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    feedbackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(feedbackSlider);
    addAndMakeVisible(feedbackLabel);
    feedbackLabel.setJustificationType(juce::Justification::centred);

    feedbackAttachment = std::make_unique<SliderAttachment>(
        audioProcessor.apvts, "feedback", feedbackSlider);

    // ---- Mix 슬라이더 ----
    mixSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(mixSlider);
    addAndMakeVisible(mixLabel);
    mixLabel.setJustificationType(juce::Justification::centred);

    mixAttachment = std::make_unique<SliderAttachment>(
        audioProcessor.apvts, "mix", mixSlider);

    setSize(420, 220);
}

void DelayAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::white);
    g.setFont(20.0f);
    g.drawFittedText("Simple Delay", getLocalBounds().removeFromTop(30),
                      juce::Justification::centred, 1);
}

void DelayAudioProcessorEditor::resized()
{
    // 3개 슬라이더를 가로로 나란히 배치하는 단순한 레이아웃
    auto area = getLocalBounds().reduced(20);
    area.removeFromTop(30); // 타이틀 자리

    const int sliderWidth = area.getWidth() / 3;

    auto delayArea    = area.removeFromLeft(sliderWidth);
    auto feedbackArea = area.removeFromLeft(sliderWidth);
    auto mixArea       = area;

    delayTimeLabel.setBounds(delayArea.removeFromTop(20));
    delayTimeSlider.setBounds(delayArea.reduced(10));

    feedbackLabel.setBounds(feedbackArea.removeFromTop(20));
    feedbackSlider.setBounds(feedbackArea.reduced(10));

    mixLabel.setBounds(mixArea.removeFromTop(20));
    mixSlider.setBounds(mixArea.reduced(10));
}
