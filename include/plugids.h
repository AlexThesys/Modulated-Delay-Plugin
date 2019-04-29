//------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Examples
// Filename    : plugids.h
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

#pragma once

#include "public.sdk/source/vst/vstparameters.h"

namespace Steinberg {
namespace MyModulation {

// HERE are defined the parameter Ids which are exported to the host
enum MyModulationParams : Vst::ParamID
{
    kParamDryWetID = 101,
    kParamModulationRateID = 102,
    kParamModulationDepthID = 103,
    kParamModWaveformID = 104,
    kParamFeedbackID = 105,
    kParamChorusOffsetID = 106,

    kParamEffectTypeID = 107,

    kBypassID = 108
};

namespace ModulationConst
{
    static constexpr double DRY_WET_MIN = 0.0;
    static constexpr double DRY_WET_MAX = 1.0;
    static constexpr double DRY_WET_DEFAULT = 0.5;
    static constexpr double FEEDBACK_MIN = -0.95;
    static constexpr double FEEDBACK_MAX = 0.95;
    static constexpr double FEEDBACK_DEFAULT = 0.4;
    static constexpr double DEPTH_MIN = 0.0;
    static constexpr double DEPTH_MAX = 1.0;
    static constexpr double DEPTH_DEFAULT = 0.5;
    static constexpr double RATE_MIN = 0.02;
    static constexpr double RATE_MAX = 5.0;
    static constexpr double RATE_DEFAULT = 0.18;
    static constexpr double CHRS_OFST_MIN = 5.0;
    static constexpr double CHRS_OFST_MAX = 35.0;
    static constexpr double CHRS_OFST_DEFAULT = 5.0;
    static constexpr int	NUM_WAVEFORMS = 4;
    static constexpr int	NUM_FX_TYPES = 3;
};


// HERE you have to define new unique class ids: for processor and for controller
// you can use GUID creator tools like https://www.guidgenerator.com/
static const FUID MyProcessorUID (0xe2c9d841, 0x22804458, 0xa5e0704a, 0x4366571a);
static const FUID MyControllerUID (0x7f24ee74, 0x55f14e51, 0xb8853578, 0xfddb071b);

//------------------------------------------------------------------------
} // namespace HelloWorld
} // namespace Steinberg



