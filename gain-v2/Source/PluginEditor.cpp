#include "PluginProcessor.h"
#include "PluginEditor.h"

//생성자
gainv2AudioProcessorEditor::gainv2AudioProcessorEditor
 (gainv2AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p) //초기화 리스트
{
    //gainSlider (.h에서 선언해둔 그 멤버 변수)
    gainSlider.setSliderStyle (juce::Slider::LinearVertical);
    gainSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20); //false-읽기전용 아님
    addAndMakeVisible (gainSlider);
    //실제로 화면에 등록하고 보이게 만들어라는 함수

    //gainLabel
    gainLabel.setText("Gain (dB)", juce::dontSendNotification);
    gainLabel.attachToComponent (&gainSlider, false);
    addAndMakeVisible (gainLabel);

    phaseInvertButton.setButtonText ("Phase Invert");
    addAndMakeVisible (phaseInvertButton);


    gainAttachment = std::make_unique<SliderAttachment> (
        audioProcessor.apvts, gainv2AudioProcessor::GAIN_DB_ID, gainSlider);

    phaseInvertAttachment = std::make_unique<ButtonAttachment> (
        audioProcessor.apvts, gainv2AudioProcessor::PHASE_INVERT_ID, phaseInvertButton);

        //SliderAttachment 타입의 객체를 새로 만들고, unique_ptr 로 감싸서 돌려

    setSize (250, 350); //Editor 창의 크기를 가로 250, 세로 350픽셀로 정해라
}

//소멸자
gainv2AudioProcessorEditor::~gainv2AudioProcessorEditor() {}

void gainv2AudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel(). findColour (juce::ResizableWindow::backgroundColourId));
}

void gainv2AudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(20);

    gainSlider.setBounds (area.removeFromTop (250).withSizeKeepingCentre (60, 250));
    area.removeFromTop(10);
    phaseInvertButton.setBounds (area.removeFromTop (30));
}


    




