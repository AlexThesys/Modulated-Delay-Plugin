#include <cmath>
#include <array>
#include <cstring>
#include "constants.h"

inline float linearInterp(float y1, float y2, float _readPoint)
{
    float scaledVal = _readPoint - std::floor(_readPoint);
    return (y1 *(1.0f - scaledVal) + (y2 * scaledVal));
} 

enum class Waveform {SINE, SAW, TRIANGLE, SQUARE};

union f_int32
{
    float f;
    int32_t i32;
};

template <size_t SIZE>
struct WTables
{
    std::array<float, SIZE> Sin;
    std::array<float, SIZE> Saw;
    std::array<float, SIZE> Tri;
    std::array<float, SIZE> Sqr;
};

template <size_t SIZE=1024u>
class WT_Osc
{
    WTables<SIZE>* wTables;
    std::array<float, SIZE>* p_wTable;
    static constexpr size_t size_mask = SIZE - 1;
    int32_t incr_i;
    float incr_f, incr_f_accum[2];
    size_t readIndex[2];
    int32_t invert;

    void reset() noexcept;
    void makeUnipolar(float*) noexcept;
public:
    WT_Osc(double);
    WT_Osc(double, const int32_t numHarmonics);   // numHarmonics = 5
    ~WT_Osc();
    void changeWaveform(Waveform) noexcept;
    void changeWaveform(int) noexcept;
    void changeFreq(double) noexcept;
    void generate(float*, int) noexcept;
    void generateQuad(float*, int) noexcept;
    void generateUnipolar(float*, int) noexcept;
    void generateQuadUnipolar(float*, int) noexcept;
    void invertPhase() { invert ^= 0x80000000; }
    void setQuadPhase() noexcept;
    void resetPhase() noexcept;
};

template <size_t SIZE>
inline void WT_Osc<SIZE>::reset() noexcept
{
    memset(readIndex, 0, 2*sizeof(size_t));
    memset(incr_f_accum, 0, 2*sizeof(float));
}

template<size_t SIZE>
inline void WT_Osc<SIZE>::setQuadPhase() noexcept
{
    constexpr size_t quaterSize = static_cast<size_t>(static_cast<double>(SIZE)* 0.25);
    readIndex[1] = (readIndex[0] + quaterSize) & size_mask;
}

template<size_t SIZE>
inline void WT_Osc<SIZE>::resetPhase() noexcept
{
    readIndex[1] = readIndex[0];
}

template <size_t SIZE>
inline void WT_Osc<SIZE>::makeUnipolar(float* buff) noexcept
{
   *buff *= 0.5f;
   *buff += 0.5f;
}

template <size_t SIZE>
WT_Osc<SIZE>::WT_Osc(double freq) : invert(0)
{
    reset();
    wTables = new WTables<SIZE>();

    constexpr float half = SIZE * 0.5f;
    constexpr float quater = SIZE * 0.25f;
    constexpr float threeQuaters = SIZE * 0.75f;
    constexpr float size_recip = 1.0f / static_cast<float>(SIZE);
    constexpr float ms = 1.0f / half;
    constexpr float b1 = 0.0f;
    constexpr float b2 = -1.0f;
    constexpr float mt = 1.0f / quater;
    constexpr float mtf = -2.0f / half;
    constexpr float btf = 1.0f;

    for (size_t j = 0; j < SIZE; ++j){
        // Sine
        wTables->Sin[j] = sinf(static_cast<float>(j) * size_recip * 2.0f * static_cast<float>(PI));
        // Saw
        wTables->Saw[j] = j < half ? (ms*j + b1) : (ms*(j-(half-1)) + b2);
        //Triangle
        if (j < quater)
            wTables->Tri[j] = mt*j + b1;
        else if (j >= quater && j < threeQuaters)
            wTables->Tri[j] = mtf*(j-quater) + btf;
        else
            wTables->Tri[j] = mt*(j-threeQuaters) + b2;
        //Square
        wTables->Sqr[j] = j < half ? 1.0f : -1.0f;
    }

    p_wTable = &wTables->Sin;   // default
    changeFreq(freq);
}

template <size_t SIZE>
WT_Osc<SIZE>::WT_Osc(double freq, int32_t numHarmonics) : invert(0)
{
    reset();
    wTables = new WTables<SIZE>();

    constexpr float size_recip = 1.0f / static_cast<float>(SIZE);
    float maxSaw = 0.0f;
    float maxTri = 0.0f;
    float maxSqr = 0.0f;

    for (size_t j = 0; j < SIZE; ++j){
        // Sine
        wTables->Sin[j] = sinf(static_cast<float>(j) * size_recip * 2.0f * static_cast<float>(PI));

       // wTables->Saw[j] = 0.0f;
       // wTables->Tri[j] = 0.0f;
       // wTables->Sqr[j] = 0.0f;

        // saw
        for (int32_t g = 1; g <= (numHarmonics + 1); ++g){
            float n = static_cast<float>(g);
            wTables->Saw[j] += pow(-1.0, g+1)*(1.0f/n)*sinf(2.0f*PI*j*n/static_cast<float>(SIZE));
        }
        if (fabs(wTables->Saw[j]) > fabs(maxSaw)) maxSaw = wTables->Saw[j];
        // triangle
        for (int32_t g = 0; g < ((numHarmonics >> 1)+1); ++g){ // or should it be g <= ((numHarmonics / 2)+1);
            float n = static_cast<float>(g);
            wTables->Tri[j] += pow(-1.0f, n) * (1.0f / pow((2*n + 1), 2.0f)) * sinf(2.0f * PI * (2.0f*n + 1) * j / SIZE);
        }
        if (fabs(wTables->Tri[j]) > fabs(maxTri)) maxTri = wTables->Tri[j];
        // square
        for (int32_t g = 1; g <= numHarmonics; g+=2){
            float n = static_cast<float>(g);
            wTables->Sqr[j] += (1.0f / n) * sinf(2.0f*PI*j*n/SIZE);
        }
        if (fabs(wTables->Sqr[j]) > fabs(maxSqr)) maxSqr = wTables->Sqr[j];
    }

    maxSaw = 1.0f / maxSaw;
    maxTri = 1.0f / maxTri;
    maxSqr = 1.0f / maxSqr;
    for (size_t j = 0; j < SIZE; ++j){
        wTables->Saw[j] *= maxSaw;
        wTables->Tri[j] *= maxTri;
        wTables->Sqr[j] *= maxSqr;
    }

    p_wTable = &wTables->Sin;   // default
    changeFreq(freq);
}

template <size_t SIZE>
inline WT_Osc<SIZE>::~WT_Osc(){
    if (wTables) delete wTables;
}

template <size_t SIZE>
void WT_Osc<SIZE>::changeWaveform(Waveform wf) noexcept
{
    if (wf == Waveform::SINE)
        p_wTable = &wTables->Sin;
    else if (wf == Waveform::SAW)
        p_wTable = &wTables->Saw;
    else if (wf == Waveform::TRIANGLE)
        p_wTable = &wTables->Tri;
    else if (wf == Waveform::SQUARE)
        p_wTable = &wTables->Sqr;
}

template <size_t SIZE>
void WT_Osc<SIZE>::changeWaveform(int wf) noexcept
{
    switch(wf){
    case static_cast<int>(Waveform::SINE) :
        p_wTable = &wTables->Sin;
        break;
    case static_cast<int>(Waveform::SAW) :
        p_wTable = &wTables->Saw;
        break;
    case static_cast<int>(Waveform::TRIANGLE) :
        p_wTable = &wTables->Tri;
        break;
    case static_cast<int>(Waveform::SQUARE) :
        p_wTable = &wTables->Sqr;
        break;
    default:
        p_wTable = &wTables->Sin;
    }
}

template <size_t SIZE>
void WT_Osc<SIZE>::changeFreq(double freq) noexcept
{
    float incr = static_cast<float>(SIZE) * freq / static_cast<float>(audio_tools::SAMPLE_RATE);
    incr_i = std::floor(incr);
    incr_f = incr - incr_i;
}

template <size_t SIZE>
void WT_Osc<SIZE>::generate(float* buffer, int ch) noexcept
{
    size_t readIndexNext = (readIndex[ch] +1) & size_mask;

    f_int32 fi32;
    fi32.f = linearInterp(p_wTable->at(readIndex[ch]),
                                p_wTable->at(readIndexNext),
                                incr_f_accum[ch]);
    fi32.i32 ^= invert;
    *buffer = fi32.f;

    // check this part (its to wrap without branching)
    incr_f_accum[ch] += incr_f;
    int32_t incr_i_accum = std::floor(incr_f_accum[ch]);
    incr_f_accum[ch] -= static_cast<float>(incr_i_accum);

    readIndex[ch] = (readIndex[ch] + incr_i + incr_i_accum) & size_mask;
}

template <size_t SIZE>
void WT_Osc<SIZE>::generateQuad(float* buffer, int ch) noexcept
{
    size_t readIndexNext = (readIndex[ch] + 1) & size_mask;	//can be [ch][0]

    f_int32 fi32;
    fi32.f = linearInterp(p_wTable->at(readIndex[ch]),
                                p_wTable->at(readIndexNext),
                                incr_f_accum[ch]);
    fi32.i32 ^= invert;
    *buffer = fi32.f;

    // check this part (its to wrap without branching)
    incr_f_accum[ch] += incr_f;
    int32_t incr_i_accum = std::floor(incr_f_accum[ch]);
    incr_f_accum[ch] -= static_cast<float>(incr_i_accum);

    readIndex[ch] = (readIndex[ch] + incr_i + incr_i_accum) & size_mask;
}

template<size_t SIZE>
inline void WT_Osc<SIZE>::generateUnipolar(float* buffer, int ch) noexcept
{
    generate(buffer, ch);
    makeUnipolar(buffer);
}

template<size_t SIZE>
inline void WT_Osc<SIZE>::generateQuadUnipolar(float* buffer, int ch) noexcept
{
    generateQuad(buffer, ch);
    makeUnipolar(buffer);
}


