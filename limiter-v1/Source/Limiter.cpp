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
