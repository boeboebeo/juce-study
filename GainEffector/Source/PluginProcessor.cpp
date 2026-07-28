#include "PluginProcessor.h"
#include "PluginEditor.h"

// :: 앞뒤가 같으면 생성자
GainEffectorAudioProcessor::GainEffectorAudioProcessor()
    // : AudioProcessor (..) 부모 생성자 호출
	: AudioProcessor (BusesProperties()
				.withInput ("Input", juce::AudioChannelSet::stereo(), true)
                    // 안의 세개의 매개변수 (이름표, 입력이 몇인지 juce 에서 미리 정의한 값, 초기 활성화 상태
				.withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
                //method chaining. BusesProperties()로 빈 상자 만들고 그 안에 .withInput.. 호출
	apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

GainEffectorAudioProcessor::~GainEffectorAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout GainEffectorAudioProcessor::createParameterLaytout()
{
	std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
	params.push_back (std::make_unique<juce::AudioParameterFloat> (
		juce::ParameterID { GAIN_DB_ID, 1 },
		"Gain",
		juce::NormalisableRange<float> (-60.0f, 12.0f, 0.01f, 0.5f),
		0.0f,
		"dB")); // 내부의 4개 매개변수 

	params.push_back (std::make_unique<juce::AudioParameterBool> (
		juce::ParameterID { PHASE_INVERT_ID, 1 },
		"Phase Invert",
		false));
		// 위상반전 제어는 참/거짓 => Bool type 사용

	return { params.begin(), params.end() };

}

void GainEffectorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	juce::dsp::ProcessSpec spec; //ProcessSpec이라는 타입의 변수를 만드는데, 이름이 spec
	spec.sampleRate = sampleRate;	//spec 이라는 상자안 sampleRate 라는 칸에 값 넣
	spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
	spec.numChannels = static_cast<juce::uint32> (getTotalNumOutputChannels());

	gainProcessor.prepare (spec);
	gainProcessor.setRampDurationSeconds (0.02);
}

void GainEffectorAudioProcessor::releaseResources() {}
	//게인 이펙터는 딱히 정리할 자원(파일, 메모리) 없음

bool GainEffectorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
	return layouts.getMainOutputChannelSet() == layouts.getMainInputChannelSet()
		&& ! layouts.getMainOutputChannelSet().isDisabled();
}

//여기부터 processBlock - 핵심. 실제로 소리를 처리하는 함수
void GainEffectorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
	juce::ScopedNoDenormals noDenormals;

	const auto totalNumInputChannels = getTotalNumInputChannels();
	const auto totalNumOutputChannels = getTotalNumOutputChannels();

	for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
		buffer.clear (i, 0, buffer.getNumSamples());

	const float gainDb = apvts.getRawParameterValue (GAIN_DB_ID) -> load();
	const bool invert = apvts.getRawParameterValue (PHASE_INVERT_ID) -> load() > 0.5f;

	gainProcessor.setGainDecibels (gainDb);
	
	juce::dsp::AudioBlock<float> block (buffer);
	juce::dsp::ProcessContextReplacing<float> context (block);
	gainProcessor.process (context);

	if (invert)
		buffer.applyGain (-1.0f);
}

juce::AudioProcessorEditor* GainEffectorAudioProcessor::createEditor()
{
	return new GainEffectorAudioProcessorEditor (*this);
}

void GainEffectorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml (state.createXml());
	copyXmlToBinary (*xml, destData);
}

void GainEffectorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
	if (xmlState != nullptr && xmlState -> hasTagName (apvts.state.getType()))
	    apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}


juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new GainEffectorAudioProcessor();
}
