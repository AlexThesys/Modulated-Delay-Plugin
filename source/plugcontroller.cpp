//------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Examples
// Filename    : plugcontroller.cpp
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

#include "../include/plugcontroller.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/base/ustring.h"

namespace Steinberg {
namespace MyModulation {

//-----------------------------------------------------------------------------
tresult PLUGIN_API PlugController::initialize (FUnknown* context)
{
	tresult result = EditController::initialize (context);
	if (result == kResultTrue)
    {
        //---Create Parameters------------
        Vst::Parameter* param;

        //-----------------------------------
        param = new Vst::RangeParameter(USTRING("Dry/Wet"), MyModulationParams::kParamDryWetID,
                                    USTRING(""), ModulationConst::DRY_WET_MIN,
                                                       ModulationConst::DRY_WET_MAX,
                                                       ModulationConst::DRY_WET_DEFAULT);

        param->setPrecision(1);
        parameters.addParameter(param);
        //-----------------------------------
        param = new Vst::RangeParameter(USTRING("Modulation Rate"), MyModulationParams::kParamModulationRateID,
                                    USTRING("Hz"), ModulationConst::RATE_MIN,
                                                       ModulationConst::RATE_MAX,
                                                       ModulationConst::RATE_DEFAULT);

        param->setPrecision(2);
        parameters.addParameter(param);
        //-----------------------------------
        param = new Vst::RangeParameter(USTRING("Modulation Depth"), MyModulationParams::kParamModulationDepthID,
                                    USTRING(""), ModulationConst::DEPTH_MIN,
                                                       ModulationConst::DEPTH_MAX,
                                                       ModulationConst::DEPTH_DEFAULT);

        param->setPrecision(2);
        parameters.addParameter(param);
        //------------------------------------
        param = new Vst::StringListParameter(USTRING("Modulation Waveform"), MyModulationParams::kParamModWaveformID,
                                             nullptr, Vst::ParameterInfo::kIsList);
        Vst::StringListParameter* strParam = static_cast<Vst::StringListParameter*>(param);
        strParam->appendString(USTRING("Sine"));	// 0
        strParam->appendString(USTRING("Saw"));  // 1
        strParam->appendString(USTRING("Triangle")); // 2
        strParam->appendString(USTRING("Square")); // 3
        parameters.addParameter(param);
        //-----------------------------------
        param = new Vst::RangeParameter(USTRING("Feedback"), MyModulationParams::kParamFeedbackID,
                                    USTRING(""), ModulationConst::FEEDBACK_MIN,
                                                       ModulationConst::FEEDBACK_MAX,
                                                       ModulationConst::FEEDBACK_DEFAULT);

        param->setPrecision(1);
        parameters.addParameter(param);
        //-----------------------------------
        param = new Vst::RangeParameter(USTRING("Chorus Offset"), MyModulationParams::kParamChorusOffsetID,
                                    USTRING("ms"), ModulationConst::CHRS_OFST_MIN,
                                                       ModulationConst::CHRS_OFST_MAX,
                                                       ModulationConst::CHRS_OFST_DEFAULT);

        param->setPrecision(1);
        parameters.addParameter(param);
        //------------------------------------
        param = new Vst::StringListParameter(USTRING("Effect Type"), MyModulationParams::kParamEffectTypeID,
                                             nullptr, Vst::ParameterInfo::kIsList);
        strParam = static_cast<Vst::StringListParameter*>(param);
        strParam->appendString(USTRING("Flanger"));	// 0
        strParam->appendString(USTRING("Chorus"));  // 1
        strParam->appendString(USTRING("Vibrato")); // 2
        parameters.addParameter(param);
        //---------------------------------
        parameters.addParameter (STR16 ("Bypass"), nullptr, 1, 0,
                                 Vst::ParameterInfo::kCanAutomate | Vst::ParameterInfo::kIsBypass,
                                 MyModulationParams::kBypassID);
    }
    return kResultTrue;
}

//------------------------------------------------------------------------
//IPlugView* PLUGIN_API PlugController::createView (const char* name)
//{
//    // someone wants my editor
//    if (name && strcmp (name, "editor") == 0)
//    {
//        VST3Editor* view = new VST3Editor (this, "view", "plug.uidesc");
//        return view;
//    }
//    return nullptr;
//}

//------------------------------------------------------------------------
tresult PLUGIN_API PlugController::setComponentState (IBStream* state)
{
	// we receive the current state of the component (processor part)
	// we read our parameters and bypass value...
    if (!state)
        return kResultFalse;

    IBStreamer streamer (state, kLittleEndian);

    double savedParam = 0.0;

    // dry/wet
    if (streamer.readDouble (savedParam) == false)
        return kResultFalse;
    setParamNormalizedFromFile(MyModulationParams::kParamDryWetID, savedParam);

    // rate
    if (streamer.readDouble (savedParam) == false)
        return kResultFalse;
    setParamNormalizedFromFile(MyModulationParams::kParamModulationRateID, savedParam);

    // depth
    if (streamer.readDouble (savedParam) == false)
        return kResultFalse;
    setParamNormalizedFromFile(MyModulationParams::kParamModulationDepthID, savedParam);

    // depth
    if (streamer.readDouble (savedParam) == false)
        return kResultFalse;
    setParamNormalizedFromFile(MyModulationParams::kParamModWaveformID, savedParam);

    // feedback
    if (streamer.readDouble (savedParam) == false)
        return kResultFalse;
    setParamNormalizedFromFile(MyModulationParams::kParamFeedbackID, savedParam);

    // feedback
    if (streamer.readDouble (savedParam) == false)
        return kResultFalse;
    setParamNormalizedFromFile(MyModulationParams::kParamChorusOffsetID, savedParam);

    // feedback
    if (streamer.readDouble (savedParam) == false)
        return kResultFalse;
    setParamNormalizedFromFile(MyModulationParams::kParamEffectTypeID, savedParam);

    // bypass
    int32 bypassState;
    if (streamer.readInt32 (bypassState) == false)
        return kResultFalse;
    setParamNormalized (MyModulationParams::kBypassID, bypassState ? 1 : 0);

    return kResultOk;
}

tresult PlugController::setParamNormalizedFromFile(Vst::ParamID tag, Vst::ParamValue value)
{
    Vst::Parameter* pParam = EditController::getParameterObject(tag);

    return (pParam) ? setParamNormalized(tag, pParam->toNormalized(value)) : kResultFalse;
}

//------------------------------------------------------------------------
} // namespace
} // namespace Steinberg
