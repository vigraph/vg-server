//==========================================================================
// ViGraph audio modules: audio-module.h
//
// Common definitions for audio modules
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VIGRAPH_AUDIO_MODULE_H
#define __VIGRAPH_AUDIO_MODULE_H

#include "../module.h"

namespace ViGraph { namespace Module { namespace Audio {

// We follow Audacity in choosing 32-bit float and 44100 sample rate -
// high resolution without creating unnecessary load and file size
// - plus easy to read to/from WAV, CD and audio IO
using sample_t = float;           // Unsigned linear PCM -1.0..1.0
const unsigned int sample_rate = 44100;


//==========================================================================
}}} //namespaces

using namespace ViGraph::Module::Audio;

#endif // !__VIGRAPH_AUDIO_MODULE_H
