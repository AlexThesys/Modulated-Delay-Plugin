//------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Examples
// Filename    : plugprocessor.cpp
// Created by  : Steinberg, 01/2018
// Description : HelloWorld Example for VST 3
//
//-----------------------------------------------------------------------------
// LICENSE
// (c) 2018, Steinberg Media Technologies GmbH, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "../include/plugprocessor.h"
#include "../include/plugids.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/base/ustring.h"
#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"

namespace Steinberg {
namespace MyModulation {

void processAudio32(Vst::ProcessData &data, int32 numChannels, PlugProcessor* processor);
void processAudio64(Vst::ProcessData &data, int32 numChannels, PlugProcessor* processor);
//-----------------------------------------------------------------------------
PlugProcessor::PlugProcessor () : m_pMod(nullptr),
                                  mDryWet(ModulationConst::DRY_WET_DEFAULT),
                                  mModRate(ModulationConst::RATE_DEFAULT),
                                  mModDepth(ModulationConst::DEPTH_DEFAULT),
                                  mFeedback(ModulationConst::FEEDBACK_DEFAULT),
                                  mChorusOffset(ModulationConst::CHRS_OFST_DEFAULT),
                                  mWaveform(0),
                                  mEffectType(0),
                                  mBypass(false)
{
	// register its editor class
    setControllerClass (MyControllerUID);
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API PlugProcessor::initialize (FUnknown* context)
{
	//---always initialize the parent-------
	tresult result = AudioEffect::initialize (context);
	if (result != kResultTrue)
		return kResultFalse;

	//---create Audio In/Out buses------
	// we want a stereo Input and a Stereo Output
	addAudioInput (STR16 ("AudioInput"), Vst::SpeakerArr::kStereo);
	addAudioOutput (STR16 ("AudioOutput"), Vst::SpeakerArr::kStereo);

	return kResultTrue;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API PlugProcessor::setBusArrangements (Vst::SpeakerArrangement* inputs,
                                                            int32 numIns,
                                                            Vst::SpeakerArrangement* outputs,
                                                            int32 numOuts)
{
	// we only support one in and output bus and these buses must have the same number of channels
	if (numIns == 1 && numOuts == 1 && inputs[0] == outputs[0])
	{
		return AudioEffect::setBusArrangements (inputs, numIns, outputs, numOuts);
	}
    return kResultFalse;
}

tresult PlugProcessor::canProcessSampleSize(int32 symbolicSampleSize)
{

     return (symbolicSampleSize == Vst::kSample32 || symbolicSampleSize == Vst::kSample64)
             ? kResultTrue : kResultFalse;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API PlugProcessor::setupProcessing (Vst::ProcessSetup& setup)
{
	// here you get, with setup, information about:
	// sampleRate, processMode, maximum number of samples per audio block
    audio_tools::SAMPLE_RATE = setup.sampleRate;
    audio_tools::BUFFER_SIZE = setup.maxSamplesPerBlock;
	return AudioEffect::setupProcessing (setup);
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API PlugProcessor::setActive (TBool state)
{
    Vst::SpeakerArrangement arr;
    if (getBusArrangement (Vst::kOutput, 0, arr) != kResultTrue)
        return kResultFalse;
    int32 numChannels = Vst::SpeakerArr::getChannelCount (arr);
    if (numChannels == 0)
        return kResultFalse;

    if (state) // Initialize
	{
		// Allocate Memory Here
        m_pMod = std::make_unique<Modulation>(audio_tools::SAMPLE_RATE, ModulationConst::RATE_DEFAULT);
        m_pMod->setDryWet(ModulationConst::DRY_WET_DEFAULT);
        m_pMod->setFeedback(static_cast<float>(ModulationConst::FEEDBACK_DEFAULT));
        m_pMod->setModDepth(ModulationConst::DEPTH_DEFAULT);
        m_pMod->setChorOffset(ModulationConst::CHRS_OFST_DEFAULT);
        m_pMod->setEffectType(FLANGER, mDryWet, mFeedback);

        m_isSampleSize64 = (processSetup.symbolicSampleSize == Vst::kSample64);
        if (processSetup.symbolicSampleSize == Vst::kSample64) {
            bypassFunc = bypassed64;
            procFunc = processAudio64;
        }
        else {
            bypassFunc = bypassed32;
            procFunc = processAudio32;
        }
    }
	else // Release
	{
		// Free Memory if still allocated
	}
	return AudioEffect::setActive (state);
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API PlugProcessor::process (Vst::ProcessData& data)
{
	//--- Read inputs parameter changes-----------
	if (data.inputParameterChanges)
	{
		int32 numParamsChanged = data.inputParameterChanges->getParameterCount ();
		for (int32 index = 0; index < numParamsChanged; index++)
		{
			Vst::IParamValueQueue* paramQueue =
			    data.inputParameterChanges->getParameterData (index);
			if (paramQueue)
			{
				Vst::ParamValue value;
				int32 sampleOffset;
				int32 numPoints = paramQueue->getPointCount ();
				switch (paramQueue->getParameterId ())
				{
                    case MyModulationParams::kParamDryWetID :
						if (paramQueue->getPoint (numPoints - 1, sampleOffset, value) ==
						    kResultTrue)
                            mDryWet = audio_tools::scaleRange<double>(ModulationConst::DRY_WET_MAX,
                                                                      ModulationConst::DRY_WET_MIN,
                                                                      value);
                            m_pMod->setDryWet(static_cast<float>(mDryWet));
                        break;
                    case MyModulationParams::kParamModulationRateID :
                        if (paramQueue->getPoint (numPoints - 1, sampleOffset, value) ==
                            kResultTrue)
                            mModRate = audio_tools::scaleRange<double>(ModulationConst::RATE_MAX,
                                                                      ModulationConst::RATE_MIN,
                                                                      value);
                            m_pMod->setLfoFreq(mModRate);
                        break;
                    case MyModulationParams::kParamModulationDepthID :
                        if (paramQueue->getPoint (numPoints - 1, sampleOffset, value) ==
                            kResultTrue)
                            mModDepth = audio_tools::scaleRange<double>(ModulationConst::DEPTH_MAX,
                                                                      ModulationConst::DEPTH_MIN,
                                                                      value);
                            m_pMod->setModDepth(mModDepth);
                        break;
                    case MyModulationParams::kParamModWaveformID :
                    if (paramQueue->getPoint (numPoints - 1, sampleOffset, value) ==
                            kResultTrue)
                        mWaveform = std::min<int8>((int8)(ModulationConst::NUM_WAVEFORMS * value),
                                                      ModulationConst::NUM_WAVEFORMS - 1);
                        m_pMod->setWaveform(mWaveform);
                    break;
                    case MyModulationParams::kParamFeedbackID :
                        if (paramQueue->getPoint (numPoints - 1, sampleOffset, value) ==
                            kResultTrue)
                            mFeedback = audio_tools::scaleRange<double>(ModulationConst::FEEDBACK_MAX,
                                                                      ModulationConst::FEEDBACK_MIN,
                                                                      value);
                            m_pMod->setFeedback(static_cast<float>(mFeedback));
                        break;
                    case MyModulationParams::kParamChorusOffsetID :
                        if (paramQueue->getPoint (numPoints - 1, sampleOffset, value) ==
                            kResultTrue)
                            mChorusOffset = audio_tools::scaleRange<double>(ModulationConst::CHRS_OFST_MAX,
                                                                      ModulationConst::CHRS_OFST_MIN,
                                                                      value);
                            m_pMod->setChorOffset(static_cast<double>(mChorusOffset));
                        break;
                    case MyModulationParams::kParamEffectTypeID :
                    if (paramQueue->getPoint (numPoints - 1, sampleOffset, value) ==
                            kResultTrue)
                        mEffectType = std::min<int8>((int8)(ModulationConst::NUM_FX_TYPES * value),
                                                      ModulationConst::NUM_FX_TYPES - 1);
                        m_pMod->setEffectType(mEffectType, mDryWet, mFeedback);
                    break;
                    case MyModulationParams::kBypassID :
						if (paramQueue->getPoint (numPoints - 1, sampleOffset, value) ==
						    kResultTrue)
                            mBypass = (value > 0.5);
						break;
				}
			}
		}
	}

	//--- Process Audio---------------------
	//--- ----------------------------------
	if (data.numInputs == 0 || data.numOutputs == 0)
	{
		// nothing to do
		return kResultOk;
    }

    if (data.numSamples > 0)
    {
        Vst::SpeakerArrangement arr;
        getBusArrangement (Vst::kOutput, 0, arr);
        int32 numChannels = Vst::SpeakerArr::getChannelCount (arr);

        if(mBypass){
            bypassFunc(data, numChannels);
        }
        else
        {
            // Process Algorithm
            procFunc(data, numChannels, this);
        }
    }
	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API PlugProcessor::setState (IBStream* state)
{
	if (!state)
		return kResultFalse;

	// called when we load a preset or project, the model has to be reloaded

	IBStreamer streamer (state, kLittleEndian);

    double savedParam = 0.0;
// dry/wet
    if (streamer.readDouble(savedParam) == false)
		return kResultFalse;
    mDryWet = savedParam;
// rate
    if (streamer.readDouble(savedParam) == false)
        return kResultFalse;
    mModRate = savedParam;
// rate
    if (streamer.readDouble(savedParam) == false)
        return kResultFalse;
    mModDepth = savedParam;
// Waveform type
    int8 savedParam8 = 0;
    if (streamer.readInt8(savedParam8) == false)
        return kResultFalse;
    mWaveform = savedParam8;
// feedback
    if (streamer.readDouble(savedParam) == false)
        return kResultFalse;
    mFeedback = savedParam;
// chorusOffset
    if (streamer.readDouble(savedParam) == false)
        return kResultFalse;
    mChorusOffset = savedParam;
// Effect type
    if (streamer.readInt8(savedParam8) == false)
        return kResultFalse;
    mWaveform = savedParam8;
// bypass
	int32 bypassState;
	if (streamer.readInt32 (bypassState) == false)
		return kResultFalse;
    mBypass = bypassState;

    return kResultOk;
}

//------------------------------------------------------------------------i
tresult PLUGIN_API PlugProcessor::getState (IBStream* state)
{
	// here we need to save the model (preset or project)

	IBStreamer streamer (state, kLittleEndian);
    streamer.writeDouble(mDryWet);
    streamer.writeDouble(mModRate);
    streamer.writeDouble(mModDepth);
    streamer.writeInt8(mWaveform);
    streamer.writeDouble(mFeedback);
    streamer.writeDouble(mChorusOffset);
    streamer.writeInt8(mEffectType);
    streamer.writeInt32 (mBypass ? 1 : 0);

    return kResultOk;
}

void processAudio32(Vst::ProcessData &data, int32 numChannels, PlugProcessor* processor)
{

    for (int32 channel = 0; channel < numChannels; channel++)
    {
        float* inputChannel = data.inputs[0].channelBuffers32[channel];
        float* outputChannel = data.outputs[0].channelBuffers32[channel];

        processor->processAudio(inputChannel, outputChannel, data.numSamples, channel);
    }
}

void processAudio64(Vst::ProcessData &data, int32 numChannels, PlugProcessor* processor)
{

    for (int32 channel = 0; channel < numChannels; channel++)
    {
        double* inputChannel = data.inputs[0].channelBuffers64[channel];
        double* outputChannel = data.outputs[0].channelBuffers64[channel];

        processor->processAudio(inputChannel, outputChannel, data.numSamples, channel);
    }
}

template<typename FloatType>
void PlugProcessor::processAudio(FloatType *in, FloatType *out, int numSamples, int ch)
{
    for (int32 sample = 0; sample < numSamples; ++sample)
    {
        float buffer = static_cast<float>(in[sample]);

        m_pMod->update(&buffer, ch);

        out[sample] = static_cast<FloatType>(buffer);
    }
}

//------------------------------------------------------------------------
} // namespace
} // namespace Steinberg
