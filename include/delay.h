#ifndef DELAY_H
#define DELAY_H

#include <memory>
#include <vector>
#include "constants.h"
#include <cstring>

class DelaySIMD
{
    std:: unique_ptr<std::vector<float>> delayBuffer[2];
    typedef struct {__m128 mWet, mDry, mFb;} DCoeffs;
    alignas (16) DCoeffs dCoeffs;
    size_t mReadIndex[2], mWriteIndex[2];
    size_t delay_buff_size, delay_buff_mask;
    __m128 extFB;
    size_t ms2samples(const int) const noexcept;
    template<typename Width>
    Width findNextPow2(Width v) const noexcept;
    size_t multipleOfN(size_t, size_t);
public:
    DelaySIMD(const double);
    void updateDelay(float*, const int) noexcept;
    void updateDelayExtFB(float*, const int) noexcept;
    void setOffset(const double) noexcept;
    void setDryWet(const float) noexcept;
    void setFeedback(const float) noexcept;
    __m128& getDelayedSample(const int ch) const noexcept;
    void setExternalFB(float fb) noexcept;
    void flushDelayBuffers() noexcept;
};

inline void DelaySIMD::setOffset(const double time) noexcept
{
    const int tm = static_cast<int>(time);
    mReadIndex[0] = multipleOfN(((mWriteIndex[0] - ms2samples(tm)) & delay_buff_mask), 4);
    mReadIndex[1] = multipleOfN(((mWriteIndex[1] - ms2samples(tm)) & delay_buff_mask), 4);
}

inline void DelaySIMD::setDryWet(const float dw) noexcept
{
    dCoeffs.mWet = _mm_set1_ps(dw);
    dCoeffs.mDry = _mm_set1_ps(1 - dw);
}

inline void DelaySIMD::setFeedback(const float fb) noexcept
{
    dCoeffs.mFb = _mm_set1_ps(fb);
}

inline size_t DelaySIMD::ms2samples(const int ms) const noexcept
{
    float sr = static_cast<float>(audio_tools::SAMPLE_RATE) / 1000.0f;
    return static_cast<size_t>(static_cast<float>(ms) * sr);
}

template<typename Width>
Width DelaySIMD::findNextPow2(Width v) const noexcept
{
    --v;
    for (uint32_t i = 1, maxShift = sizeof (Width) << 3; i <= maxShift; i = i << 1){
        v |= v >> i;
    }
    ++v;
    return v;
}

inline size_t DelaySIMD::multipleOfN(size_t sRate, size_t N)
{
    size_t adjust = N - (sRate & (N - 1));
       return (sRate + adjust);
}

inline __m128& DelaySIMD::getDelayedSample(const int ch) const noexcept
{
    return *(reinterpret_cast<__m128*>(&(delayBuffer[ch]->at(
                                             static_cast<size_t>(mReadIndex[ch])))));
}

inline void DelaySIMD::setExternalFB(float fb) noexcept
{
    extFB = _mm_set1_ps(fb);
}

inline void DelaySIMD::flushDelayBuffers() noexcept
{
    memset(&delayBuffer[0], 0, sizeof (float) * delay_buff_size);
    memset(&delayBuffer[1], 0, sizeof (float) * delay_buff_size);
}

//------------------------------------------------------------------------

class DelayFractional
{
    std:: unique_ptr<std::vector<float>> delayBuffer[2];
    typedef struct {float mWet, mDry, mFb;} DCoeffs;
    DCoeffs dCoeffs;
    size_t mReadIndex[2], mWriteIndex[2];
    size_t delay_buff_size, delay_buff_mask;
    float delayFraction;
    float extFB;
    size_t ms2samples(const double) noexcept;
    float linearInterp(float, float);
public:
    DelayFractional(const double);
    void updateDelay(float*, const int) noexcept;
    void updateDelayExtFB(float*, const int) noexcept;
    void setOffset(const double) noexcept;
    void setDryWet(const float) noexcept;
    void setFeedback(const float) noexcept;
    float& getDelayedSample(const int ch) const noexcept;
    void setExternalFB(float fb) noexcept;
    void flushDelayBuffers() noexcept;
};

inline void DelayFractional::setOffset(const double time) noexcept
{
    mReadIndex[0] = ((mWriteIndex[0] - ms2samples(time)) & delay_buff_mask);
    mReadIndex[1] = ((mWriteIndex[1] - ms2samples(time)) & delay_buff_mask);
}

inline void DelayFractional::setDryWet(const float dw) noexcept
{
    dCoeffs.mWet = dw;
    dCoeffs.mDry = 1.0f - dw;
}

inline void DelayFractional::setFeedback(const float fb) noexcept
{
    dCoeffs.mFb = fb;
}

inline size_t DelayFractional::ms2samples(const double ms) noexcept
{
    const float delaySamples = static_cast<float>(audio_tools::SAMPLE_RATE * ms) / 1000.0f;
    const size_t delayIntegral = static_cast<size_t>(delaySamples);
    delayFraction = delaySamples - static_cast<float>(delayIntegral);
    return delayIntegral;
}


inline float& DelayFractional::getDelayedSample(const int ch) const noexcept
{
    return delayBuffer[ch]->at(mReadIndex[ch]);
}

inline void DelayFractional::setExternalFB(float fb) noexcept
{
    extFB = fb;
}

inline void DelayFractional::flushDelayBuffers() noexcept
{
    memset(&delayBuffer[0], 0, sizeof (float) * delay_buff_size);
    memset(&delayBuffer[1], 0, sizeof (float) * delay_buff_size);
}

inline float DelayFractional::linearInterp(float y0, float y1)
{
    return (y0 *(1.0f - delayFraction) + (y1 * delayFraction));
}
#endif // DELAY_H

