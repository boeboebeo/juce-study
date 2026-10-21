/*
    Limiter.cpp
 */

#include "Limiter.h"

//SlidingMinimum
void SlidingMinimum::prepare(int newWindowSize)
{
    windowSize = juce::jmax (1, newWindowSize); //윈도우 크기가 최소 1은 되게 함
    capacity = windowSize + 1;
        //worst case : widowSize + 1 candidates before expiry
        //내부 자료구조가 처리중에 필요한 여유공간 확보 (새후보를 먼저 추가하고, 오래된 후보를 나중에 제거하는 순서라면 잠깐 동안 후보가 6개 일 수 있음)
    
    values.assign ((size_t) capacity, 1.0f);
        //values = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}
        //최솟값 후보가 가진 실제값 저장
    indices.assign ((size_t) capacity, 0);
        // indices = {0, 0, 0, 0, 0, 0}
        //그 후보가 들어온 샘플의 인덱스 저장
    reset();
}

void SlidingMinimum::reset()
{
    head = 0; //후보중 가장 앞에 있는 값의 위치
    count = 0;  //현재 관리하고 있는 후보의 개수
    sampleCounter = 0;  //현재 샘플의 번호
}

float SlidingMinimum::process (float x)
//const float windowMin = slidingMin.process(releaseState); 이렇게 호출받아 쓸 예정
{
    while (count > 0) //현재관리하고 있는 후보의 개수가 > 0이라면 마지막 후보의 위치를 찾음
    {
        const int back = (head + count - 1) % capacity;
        //모듈로 : 배열의 끝을 넘어가면 처음으로 다시 돌아오게 하기 위해서
        
        if (values[(size_t) back] >= x)
            --count;
            //values 덱 안의 마지막 후보(back)가 새 값보다 크거나 같은지 확인
            //더 크다면 count 를 1줄여서 마지막 후보를 덱에서 제외함
            //그리고 그 자리에 새 값 x 를 저장하면 됨
            //마지막 값과 비교 후 -> count -1 -> 또 그 앞과 비교후 -> count -1 ..(새 값 보다 튼 뒤쪽 후보를 하나씩 제외)
            //반복하다가 마지막 값보다 x 가 크면 break 하고 마지막에 새 x 를 추가함
        else
            break;
    }
    
    //2. push the new value.
    const int pos = (head + count) % capacity;
        //덱의 마지막 후도 다음에 사용할 위치, 새 값 저장할 위치
    
    values[(size_t) pos] = x;   //그 위치에 x 저장
    indices[(size_t) pos] = sampleCounter; //샘플 번호 저장
    ++count;    //새로운 후보 추가되었으니 현재 관리하는 후보의 개수를 하나 늘림
    
    
    //3.drop the front if it has slid out of the window(너무 오래되어서 현재 윈도우 밖으로 밀려난 후보를 앞에서 제거)
    while (indices[(size_t) head] <= sampleCounter - windowSize)
    {
        head = (head + 1) % capacity; //head 를 옮기면 그 앞의 위치의 값은 더이상 현재 덱의 후보로 취급되지 않음
        --count;
    }
    
    ++sampleCounter //다음 샘플 처리할 준비 (그 다음 호출에서는 다음 샘플 처리)
    
    //the front of the deque is the minimum of the current window
    return values[(size_t) head];
        //새로운 값보다 크거나 같은 뒤쪽 후보는 제거함
        //현재 윈도우에서 만료된 앞쪽 후보들도 제거함
        //값은 오름차운으로 저장되어있으므로, 맨 앞의 후보가 현재 윈도우 내에서의 최솟값이 되는것 - 그걸 windowMin 에 저장함
}



//Limiter
void Limiter::prepare(double sampleRate, int /*maximumBlockSize*/, int newNumChannels)
{
    fs = sampleRate;
    numChannels = juce::jmax (1, newNumChannels);
    lookaheadSamples = juce::jmax (1, (int) std::ceil ((double) lookaheadMs * 0.001 * fs));
        //lookahead를 초로 샘플 개수로 계산
    
    const int windowSize = lookaheadSamples + 1;
        //현재 샘플까지 포함하기 위해서 +1 처리함
    
    delayBuffers.assign ((size_t) numChannels, std::vector<float> ((size_t) windowSize, 0.0f));
    averageBuffer.assign ((size_t) windowSize, 1.0f);
    slidingMin.prepare (windowSize);
        //위에서 정의한 slidingMin 클래스
    
    inputGainSmoothed.reset(fs, 0.02);
        //20ms ramp for input gain changes
        //.reset(fs, 0.02) : 램프시간 20ms 를 기준으로 스무딩을 설정함
    inputGainSmoothed.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (inputGainDb));
        //inputGainDb는 사용자가 입력하는 인풋 레벨 
    
    ceilingLinear = juce::Decibels::decibelsToGain (ceilingDb);
    updateReleaseCoefficient();
    
    reset();
    //이건 slidingMimium 안에 있는 reset()말고,
    //Limiter class 안에 있는 reset() 이 호출되는것
    //this->reset()
}

void Limiter::reset()
{
    for (auto& buffer : delayBuffers)
        std::fill (buffer.begin(), buffer.end(), 0.0f);
    //아까위에서 0.0f 으로 리셋해놓은 delayBuffers 벡터를 사용하다가..
    //다시 초기화할때 씀
    //prepare() : 버퍼를 필요한 크기로 준비 및 초기화,
    //reset() : 이미 존재하는 버퍼와 내부 상태를 다시 초기화.
    
    delayWritePos = 0;
        //지연버퍼의 위치 (다음 오디오 샘플을 기록할 위치)
        //delayBuffers 에서의 위치 -> lookahead 때문에 오디오를 잠시 저장해놨다가 출력해야 해서 사용함
    //기록할때마다 위치를 다음 칸으로 옮김 -> 마지막칸 다음은 다시 처음
    
    releaseState = 1.0f;
    slidingMin.reset();
    
    std::fill (averageBuffer.begin(), averageBuffer.end(), 1.0f);
        //averageBuffer 는 오디오 샘플이 아니라, 게인값을 저장하는 버퍼이기 때문에
        //1.0f 으로 초기화 => 원래 신호 유지한다는 뜻 
    averagePos = 0;
    averageSum = (double) averageBuffer.size();
        //averageBuffer 에는 그렇게 계산된 windowMin이 들어가게 됨
    //입력신호의 크기와 ceiling 비교 -> Release 및 최솟값 계산을 거쳐서 얻은 게인값
    //.size() 함수의 반환값을 double 로 변환해서 averageSum 에 대입
    //.size() 는 벡터안에 지금 몇개가 있는 지 알려줌 (그거로 평균내려고)
    
    currentGrDb.store(0.0f);
        //currentGrDb : 현재 게인 감소량을 저장하는 변수에
        //0.0f를 저장하는 것
        //GrDb = Gain Reduction in dB (Linear 값 아님)
}


void Limiter::setInputGain (float newInputGainDb)
{
    inputGainDb = newInputGainDb;
    inputGainSmoothed.setTargetValue (juce::Decibels::decibelsToGain (inputGainDb));
}

void Limiter::setCeiling (float newCeilingDb)
{
    ceilingDb = newCeilingDb;
    ceilingLinear = juce::Decibels::DecibelsToGain (ceilingDb);

}

void Limiter::setRelease(float newReleaseMs)
{
    releaseMs = juce::jmax (1.0f, newReleaseMs);
    updateReleaseCoefficient();
}

void Limiter::updateReleaseCoefficient()
{
    
}
