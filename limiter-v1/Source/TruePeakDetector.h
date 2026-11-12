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
    
#ifndef TP_HALF_LENGTH
    static constexpr int halfLengthSamples = 12;
#else
    static constexpr int halfLengthSamples = TP_HALF_LENGTH;
#endif
#ifndef TP_KAISER_BETA
    static constexpr double kaiserBeta = 8.0;
#else
    static constexpr double kaiserBeta = TP_KAISER_BETA;
#endif
}
