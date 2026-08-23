/*
    PluginEditor.h

    화면에 보이는 UI. 슬라이더 3개(Delay Time, Feedback, Mix)만 있는 단순한 형태.

    SliderAttachment란?
    - 슬라이더를 움직이면 apvts의 파라미터 값이 자동으로 갱신됨
    - 반대로 automation이나 프리셋으로 파라미터가 바뀌면 슬라이더도 자동으로 따라 움직임
    - 즉 "슬라이더 값 -> setXXX() 호출" 코드를 직접 안 짜도 됨
      (PluginProcessor::updateSmoothedTargets()가 apvts 값을 읽어가는 구조라서)
*/

#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

class DelayAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit DelayAudioProcessorEditor(DelayAudioProcessor&);
    ~DelayAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    DelayAudioProcessor& audioProcessor; // 이 Editor가 보고 있는 Processor (파라미터 접근용)

    juce::Slider delayTimeSlider;
    juce::Slider feedbackSlider;
    juce::Slider mixSlider;

    juce::Label delayTimeLabel { {}, "Delay Time" };
    juce::Label feedbackLabel   { {}, "Feedback" };
    juce::Label mixLabel         { {}, "Mix" };

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> delayTimeAttachment;
    std::unique_ptr<SliderAttachment> feedbackAttachment;
    std::unique_ptr<SliderAttachment> mixAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DelayAudioProcessorEditor)
};
