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
    
    ++sampleCounter; //다음 샘플 처리할 준비 (그 다음 호출에서는 다음 샘플 처리)
    
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
    ceilingLinear = juce::Decibels::decibelsToGain (ceilingDb);

}

void Limiter::setRelease(float newReleaseMs)
{
    releaseMs = juce::jmax (1.0f, newReleaseMs);
    updateReleaseCoefficient();
}

void Limiter::updateReleaseCoefficient()
    //게인 회복속도 계산 (Release 계수 계산)
{
    // One-pole time constant : the gain recovers ~ 63% of the way back toward its target after 'releaseMs' milliseconds.
    releaseCoeff = std::exp (-1.0f / (0.001f * releaseMs * (float) fs));
        //exp : 자연상수 e의 x제곱을 계산. exp(x) = e^x
    
}


void Limiter::processBlock (juce::AudioBuffer<float>& buffer)
    //여러 샘플을 묶은 오디오 블록 단위로 전달받음
    //블록 내부에서는 각 샘플을 하나씩 순차적으로 처리함
{
    const int channels = juce::jmin (buffer.getNumChannels(), numChannels);
    const int numSamples = buffer.getNumSamples();
    const int windowSize = lookaheadSamples + 1;
    
    float blockMinGain = 1.0f;
        //이 블록에서 가장 작았던 게인 값을 기록해두는데 사용
    
    for (int i = 0; i < numSamples; ++i)
        //샘플 0번 : channel 1 -> 2
        //샘플 1번 : channel 1 -> 2 이렇게 한 샘플당 각 채널을 확인후, 하나의 peak 계산
    {
        const float inputGain = inputGainSmoothed.getNextValue();
            //스무딩 시간에 따른 다음 샘플에 적용할 게인 값을 계산해서 반환함
        
        //1. Apply input gain,
        float peak = 0.0f;
        
        for (int ch = 0; ch < channels; ++ch)
        {
            const float x = buffer.getWritePointer (ch)[i] * inputGain;
                //입력게인을 적용한 샘플 만들기
                //현재 채널의 i번째 샘플을 가져옴
            
            delayBuffers[(size_t) ch][(size_t) delayWritePos] = x;
                //이중 배열 [채널번호][writePos(지정된 위치)]
                //그 지정된 위치에 x값(게인 적용된 값)을 보관함
            peak = juce::jmax (peak, std::abs(x));
                //기존 peak(위에있는 변수)와 현재 샘플의 절댓값 중 큰 값을 선택
                //L, R 중 큰 값을 선택하기 위해서 (Stereo-linked Peak detection)
            
        }
        
        //2. Required gain g[n]:
        const float required = (peak > ceilingLinear) ? (ceilingLinear / peak) : 1.0f;
            //조건 ? 참일 때 값 : 거짓일 때 값; (삼항연산자)
            //현재 피크를 ceiling 이하로 만드려면 입력신호에 ceilingLinear/peak 를 곱해줘야 함 => 그래서 ceiling 값이 되니까
            //이미 ceiling 이하라면 감쇄할 필요 x -> 1.0반환
        
            //peak * required = ceilingLienar
            //따라서 required = ceilingLinear/peak
            //이렇게 하면 현재 피크가 정확히 ceiling 에 도달하게 됨
        
        //3. Release stage r[n]
        //더 많이 감쇄해야 하는 경우
        if (required < releaseState)
            releaseState = required;
        //감쇄량을 같거나 줄여도 되는 경우 - 이제 조금씩 압축했던걸 회복함
        //release 계수를 이용해 목표게인으로 서서히 회복
        else
            releaseState = required + releaseCoeff * (releaseState - required);
        
        //4. Sliding minimum over the lookahead window
        //최근 lookahead 윈도우 안의 releaseState 중 최솟값을 구함
        const float windowMin = slidingMin.process (releaseState);
        
        //5. Moving average 처리
        averageSum += (double) windowMin - (double) averageBuffer[(size_t) averagePos];
        
        averageBuffer[(size_t) averagePos] = windowMin;
            //새 값을 버퍼에 저장
        
        averagePos = (averagePos + 1) % windowSize;
            //다음 위치로 이동하기 (원형버퍼를 사용하기 때문에 마지막 위치에 도달하면 다시 0으로 돌아감
        
        const float smoothedGain = (float) (averageSum / (double) windowSize);
            //전체 합계를 버퍼 크기로 나눔
            //averageBuffer 에 저장된 게인 값들의 산술평균 계산
            //평균값이 반드시 windowMin 이하인 것은 아님

        blockMinGain = juce::jmin (blockMinGain, smoothedGain);
            //블록에서 가장 작은 게인 기록
            //현재 샘플에서 계산한 smooothedGain 과 지금까지 기록한 blockMinGain 중 더 작은 값을 선택함
        
        //6.
        const int readPos = (delayWritePos + 1) % windowSize;
            //Lookahead 지연 버퍼에서 오디오 읽기
            //delayWritePos(이번 샘플을 기록한 위치) 와 readPos(지연 버퍼에서 출력할 샘플을 읽을 위치) 구분
            //lookahead 샘플 수 만큼 오디오가 지연됨
        
            //현재 기록위치의 다음칸을 읽기 위치로 지정함
            //원형 버퍼에서 가장 오래된 샘플을 읽어오기 위한 위치
            //결과적으로 lookahead 샘플 수 만큼 지연된 오디오를 읽음
        
        for (int ch = 0; ch < channels; ++ch)
        {
            const float delayed = delayBuffers[(size_t) ch][(size_t) readPos];
                //앞에서 저장해 놓았던 오디오 샘플 읽어오기
                //Lookahead 만큼 이전에 입력된 샘플(1.번 과정에서 입력해둠 .. = x 로)
            buffer.getWritePointer (ch)[i] = juce::jlimit (-ceilingLinear, ceilingLinear, delayed * smoothedGain);
        }
            //delayed * smoothedGain : 게인 적용
            //lookahead 샘플 개수만큼 전에 입력한 delayBuffers안의 샘플에다가 smoothedGain을 곱해줌
            //juce::jlimit => 최종 출력 제한
            //넘어가면은 하드 클리핑에 가까운 처리를 함
        
        delayWritePos = readPos;
            //다음 샘플을 위한 기록 위치 이동
        
    }
    
    currentGrDb.store (juce::Decibels::gainToDecibels (blockMinGain, -60.0f));
        //현재 오디오 블록의 모든 샘플을 처리한 다음에 한번 실행됨
        //-60.0f : 게인이 0일때 사용할 dB 값 지정 . 로그 계산에서 0은 표현할 수 없기 때문에 대신 -60.0f 를 반환함
        //.store -> GUI 에서 게인 감소량 미터 보여줄때 활용

}

/*
 next step
    
    - true-peak meter 추가
    - 
 */
