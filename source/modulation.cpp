#include "../include/modulation.h"

Modulation::Modulation(const double sr, const double freq)
{
    m_pDelay = std::make_unique<DelayFractional>(sr);
    m_pLFO = std::make_unique<WT_Osc<1024>>(freq);
}

void Modulation::update(float* buffer, const int ch) noexcept
{
    calculateDelayOffset(ch);
    m_pDelay->updateDelay(buffer, ch);
}

void Modulation::setEffectType(const int fxT, const double dw, const double fb) noexcept
{
    switch (fxT) {
        case FLANGER :
        m_deltaDelayTime = 7.0f;
        m_chorusMask = 0x0;
        m_pDelay->setDryWet(static_cast<float>(dw));
        m_pDelay->setFeedback(static_cast<float>(fb));
        break;
        case CHORUS:
            m_deltaDelayTime = 25.0f;
            m_chorusMask = ~0x0;
            m_pDelay->setDryWet(static_cast<float>(dw));
            m_pDelay->setFeedback(static_cast<float>(fb));
            break;
        case VIBRATO :
        m_deltaDelayTime = 7.0f;
        m_pDelay->setDryWet(1.0f);
        m_pDelay->setFeedback(0.0f);
        m_chorusMask = 0x0;
        break;
    default:
        m_deltaDelayTime = 7.0f;
        m_chorusMask = 0x0;
    }
}

