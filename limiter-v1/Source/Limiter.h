/*
 Limiter.h
 
 Signal flow (per sample):
    
    input * inputGain
        |
        +--> delay line (lookahead) -------------------------+
        |                                                    |
        +--> stereo-linked peak detector                     |
        |
        required gain g[n] = min(1, ceiling/peak)
        |
        release stage r[n] = (instant down, exponential up)
        |
        sliding MINIMUM over the lookahead window m[n]
            // 큰 피크를 제한하기 위해 gain 을 0.5까지 낮췄다면,
            // 그 피크가 실제 출력될때까지 gain 이 너무 빨리 회복되지 않도록 낮은 값 유지함
            // (release 때문에 gain 이 회복되기 시작하면 lookahead 라는 delay 때문에 시점이 다름, 그 딜레이 샘플들 각각의 g[n] 중 제일 작은 걸 선택)
        |
        moving AVERAGE over the lookahead window s[n]-------+
            // gain 변화를 부드럽게 만듦
                                                            |
                                                            |
                                                         ouput (<-ceiling)
 
    lookahead : L
    - The audio is delayed by L samples,
    - 출력시점 n 에 나오는 오디오는 L 의 영향으로 과거샘플 x[n - L]임.
 
    the gain ramp is spread over(-에 분산하다) the lookahead window, the
    attack is "as fast as the lookahead allows" and is not a separate user parameter.
 
 */


#pragma once

#include <JuceHeader.h>
#include <atomic>
#include <vector>


// O(1) amortised sliding-window minimum (monotonic deque in a ring buffer).
// 샘플당 amortised O(1) -> 전체작업량은 입력 샘플 수 에 비례함 => 전체작업량 = O(N)
// monotonic deque : 양쪽 끝에서 넣고 빼는게 가능한 자료구조.
// 근데 monotonic 이기 때문에 값의 순서를 유지함
class SlidingMinimum
{
public:
    //멤버 함수
    void prepare (int newWindowSize);
    void reset();
    float process (float x);
        //.cpp 파일 내부에서 slidingMin.process(releaseState)라는 함수를 사용함
        //거기서의 releaseState 가 곧 매개변수 x가 되는 것
    
    
private:
    //멤버변수들
    int windowSize = 1;
    int capacity = 2;
    int head = 0;   //position of the front (oldest, smallest) element
    int count = 0;  //number of elements currently stored
    juce::int64 sampleCounter = 0;
    
    std::vector<float> values;
    std::vector<juce::int64> indices;   //지수(거듭제곱)
};


class Limiter
{
public:
    static constexpr float lookaheadMs = 5.0f;
    
    void prepare (double sampleRate, int maximumBlockSize, int numChannels);
    void reset();
    
    void setInputGain (float newInputGainDb);
    void setCeiling (float newCeilingDb);
    void setRelease (float newReleaseMs);
    
    //fixed after prepare(); 호스트 DAW에 getLatencySamples()를 통해 알려줘야 함
    int getLatencySamples() const noexcept { return lookaheadSamples; }
        //Limiter::prepare 내부에 내용있음 (Limiter.cpp)
    
    void processBlock (juce::AudioBuffer<float>& buffer);
    
    //최근 처리한 오디오블록중에서 가장 강한 gain reduction 값을 dB(음수)로 변환함
    float getCurrentGainReductionDb() const noexcept { return currentGrDb.load(); }
        //currentGrDb 값을 원자적인 변수(currentGrDb) 상태로 읽어옴
        //.. 이 코드 다시 보기 
    
}
