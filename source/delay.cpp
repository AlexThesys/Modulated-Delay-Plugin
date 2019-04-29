#include "../include/delay.h"

DelayFractional::DelayFractional(const double sr) : extFB(0.0f)
{
    delay_buff_size = findNextPow2(static_cast<size_t>(sr*2.0));	// sr*2 = 2 sec
    delay_buff_mask = delay_buff_size - 1;
    delayBuffer[0] = std::make_unique<std::vector<float>>(delay_buff_size);
    delayBuffer[1] = std::make_unique<std::vector<float>>(delay_buff_size);
    memset(&dCoeffs, 0, sizeof(DCoeffs));
    memset(mWriteIndex, 0, sizeof (size_t)*2);
    memset(delayFraction, 0, sizeof (float)*2);
}

void DelayFractional::updateDelay(float* buffer, const int ch) noexcept
{
    const float xn = *buffer;
    float yn = 0.0f;
    calculateYn(xn, yn, ch);
    delayBuffer[ch]->at(mWriteIndex[ch]) = xn + yn * dCoeffs.mFb;
    *buffer = dCoeffs.mDry * xn + dCoeffs.mWet * yn;

    updateIndices(ch);
}

void DelayFractional::updateDelayCrossFB(float* buffer, const int ch) noexcept
{
    const float xn = *buffer;
    float yn = 0.0f;
    calculateYn(xn, yn, ch);
    delayBuffer[ch ^ 0x1]->at(mWriteIndex[ch ^ 0x1]) = xn + yn * dCoeffs.mFb;
    *buffer = dCoeffs.mDry * xn + dCoeffs.mWet * yn;

    updateIndices(ch);
}

void DelayFractional::updateDelayExtFB(float* buffer, const int ch) noexcept
{
    const float xn = *buffer;
    float yn = 0.0f;
    calculateYn(xn, yn, ch);
    delayBuffer[ch]->at(mWriteIndex[ch]) = xn + extFB;
    *buffer = dCoeffs.mDry * xn + dCoeffs.mWet * yn;

    updateIndices(ch);
}
