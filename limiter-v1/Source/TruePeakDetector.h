/*
    TruePeakDetector.h
 
    estimates the "true peak (inter-sample peak)" of a signal by 4x oversampling with a polyphase FIR interpolator,
    -> then taking the largest absolute value among the interpolated points.
 
    //Polyphase - 하나의 긴 FIR 필터를 여러개의 작은 필터(Phase)로 나누어 계산하는 방식
 
    => FIR Filter의 계수가
        : BS.1770-3의 Annex 2 여기에 제시되어있음
         (48차(order 48), 4-phase FIR 보간 필터의 계수표 - 4배 오버샘플링을 위한 네개의 계수 그룹을 의미)
        // 여기서 48차 (order 48)은 일반적으로 49개의 탭을 가진 FIR 을 의미함
     
     https://www.itu.int/dms_pubrec/itu-r/rec/bs/R-REC-BS.1770-5-202311-I!!PDF-E.pdf
        // 18p.
 
    **현재 이 코드에서는 계수를 직접 생성하지만, 실제 ITU 예시에서는 문서에 제시된 계수를 사용. 그리고 이 코드에서는 97 taps 를 사용하지만,
        -> ITU 예시에서는 48차, 4-phase 구현을 사용
            (이미 계산된 계수값을 표로 제공하는데, 그걸 사용함)
     
 */

#pragma once

#include <algorithm>
#include <array>
#include <cmath>

class TruePeakDetector
{
public:
    static constexpr int oversampling = 4;
    
    //일반적인 if 문은 프로그램이 실행될때 조건을 검사하지만
    //ifndef는 컴파일이 시작되기 전처리단계에서 검사함
#ifndef TP_HALF_LENGTH  //if not defined (정의되어있지 않다면 아래 코드 사용)
    static constexpr int halfLengthSamples = 12;
#else                   //정의되어있다면 아래코드 사용
    static constexpr int halfLengthSamples = TP_HALF_LENGTH;
#endif                  //조건부 전처리 구간을 끝냄
#ifndef TP_KAISER_BETA
    static constexpr double kaiserBeta = 8.0;
#else
    static constexpr double kaiserBeta = TP_KAISER_BETA;
#endif
}

static constexpr int numTaps = oversampling * 2 * halfLengthSamples + 1;
    //97 for 12
static constexpr int tapsPerPhase = 2 * halfLengthSamples + 1 ;
    //phase 0 uses a 11, others one less
static constexpr int int groupDelaySamples = halfLengthSamples;
    //(numTaps-1) / 2 / 4 (2로 나누고, 4로 나누면 지연 샘플이 나옴)
    //원래 지연시간이 D = (N - 1) / 2 였으니까 (대칭 FIR 필터의 그룹지연)
    //ex. D = (97-1) / (2*4) = 12 samples


    TruePeakDetector()
{
        const auto h = makePrototype();
        //밑에서 정의되는 함수 -> h(Kaiser-windowed sinc 방식으로 FIR 필터의 계수 97개를 계산해서 반환) 를 반환한다.
        //makePrototype() 을 호출하면 계산된 97개의 계수 배열이 돌아오는 것
        
        // Phase-major layout: coeffs[p][k] = h[4k + p]
        for (int p = 0; p < oversampling; ++p)
            for (int k = 0; k < tapsPerPhase; ++k)
            {
                const int j = oversampling * k + p;
                //j =
                coeffs[(size_t) p][(size_t) k] = (j < numTaps) ? (float) h[(size_t) j] : 0.0f;
            }
        
        reset();
    }

    void reset()
    {
        history.fill (0.0f);
            //아래에서 선언한 histroy array -> 초기화 (position 도 초기화)
            //FIR 연산에서는 과거 입력도 참조하므로 <- 처음 필터를 실행할때는 과거 샘플이 존재하지 않으므로 과거 샘플이 없다는 것을 표현하기 위해 0으로 초기화
        pos = 0;
    }

// Feed one sample : returns the largest |value| among the 4 interpolated points that belong to step
// they sit at waveform times (n - 12) + p/4, p = 0..3)
inline float process (float x) noexcept
    //x값이 입력되면 (하나의 샘플 값)
    //inline : 헤더 파일에 함수 정의를 작성할때 유용
{
    pos = (pos + 1) & historyMask;
        // &: AND 연산자 (비트 AND 연산자)
        // 32칸(아래에서 historyMask = 32라고 써놓음) Ring buffer 의 순환구조
            // c++ &(AND) 연산 : bit AND 연산이라서, 십진수 숫자를 이진수로 변환하고, 각 2진수를 AND 연산하면 그 buffer 크기 초과하는 값들은 모듈로 연산처럼 ring buffer 처리될 수 있게 됨
    
    history[(size_t) pos] = x;
    
    float peak = 0.0f;
        //피크 저장 변수
        //가 phase 의 계산 결과중 가장 큰 절대값을 peak 에 저장할것임 (지금은 초기화)
    
    for (int p = 0; p < ovresampling; ++p)
    {
        float acc = 0.0f;
            //초기화 (acc = acuumulator)
            //accumulator : FIR 필터는 여러개의 곱셈 결과를 더해서 하나의 출력값을 만듦 -> 그 합계를 임시로 저장하는 변수
        
        for (int k = 0; k < tapsPerPhase; ++k)
            acc += coeffs[(size_t) p][(size_t) k] * history[(size_t) ((pos-k) & historyMask)];
            //coeffs[p][k] : 현재 Phase 의 FIR 계수
            //history[(pos-k) & historyMask] : 현재 위치에서 과거로 k만큼 떨어진 입력샘플
        
        const float a = std::abs(acc);
        peak = (a > peak) ? a : peak;
            //새로 계산한 절댓값 a 가 기존 Peak 보다 크면 a를 저장한다.
            //아니면 기존 peak 값 유지
    }
    
    return peak;
    
}
