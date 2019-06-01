#ifndef MODULATION_H
#define MODULATION_H

#include "delay.h"
#include "WT_Osc.h"

enum FxType {FLANGER, VIBRATO, CHORUS};
union F_I_32 {float f; int32_t i;};

class ModFilter
{
    std::unique_ptr<DelayFractional> m_pDelay;
    std::unique_ptr<WT_Osc<1024>> m_pLFO;
    float m_deltaDelayTime, m_chorusOffset, m_modDepth;
    int32_t m_chorusMask = 0x0;
public:
    ModFilter(const double sr, const double freq);
    void update(float*, const int) noexcept;
    void setEffectType(const int, const double, const double) noexcept;
    void calculateDelayOffset(const int ch) noexcept;
    void setDryWet(const float) noexcept;
    void setFeedback(const float) noexcept;
    void setWaveform(const int) noexcept;
    void setLfoFreq(const double) noexcept;
    void setChorOffset(const double) noexcept;
    void setModDepth(const double modDepth) noexcept;
    void toggleQuadPhase(bool) noexcept;
};

inline void ModFilter::setDryWet(const float dw) noexcept
{
    m_pDelay->setDryWet(dw);
}

inline void ModFilter::setFeedback(const float fb) noexcept
{
    m_pDelay->setFeedback(fb);
}

inline void ModFilter::setWaveform(const int wf) noexcept
{
    m_pLFO->changeWaveform(wf);
}

inline void ModFilter::setLfoFreq(const double f) noexcept
{
    m_pLFO->changeFreq(f);
}

inline void ModFilter::setChorOffset(const double chrsOffst) noexcept
{
    m_chorusOffset = static_cast<float>(chrsOffst);
}

inline void ModFilter::setModDepth(const double modDepth) noexcept
{
    m_modDepth = static_cast<float>(modDepth);
}

inline void ModFilter::toggleQuadPhase(bool onOff) noexcept
{
    onOff ? m_pLFO->setQuadPhase() : m_pLFO->resetPhase();
}

inline void ModFilter::calculateDelayOffset(const int ch) noexcept
{
    float lfoSampleVal = 0.0f;
    m_pLFO->generateUnipolar(&lfoSampleVal, ch);

    constexpr float min_delay = 0.01f;

    F_I_32 fi32;
    fi32.f = m_chorusOffset;
    fi32.i &= m_chorusMask;
    fi32.f += m_modDepth * lfoSampleVal * m_deltaDelayTime + min_delay;
    m_pDelay->setOffset(static_cast<double>(fi32.f), ch);
}

#endif // MODULATION_H
