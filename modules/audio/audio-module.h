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
using sample_t = float;           // Unsigned linear PCM 0.0..1.0
const int sample_rate = 44100;
using multi_sample_t = vector<sample_t>;  // Multiple channel samples

//==========================================================================
// Audio frame
struct Frame: public Data
{
  timestamp_t timestamp;
  int nchannels;
  vector<multi_sample_t> waveform;

  Frame(timestamp_t t, int _nchannels=1):
    timestamp(t), nchannels(_nchannels) {}
};

using FramePtr = shared_ptr<Frame>;

//==========================================================================
// Specialisations of Dataflow classes for Frame data
class FrameFilter: public Filter
{
 public:
  // Construct with XML
  FrameFilter(const Dataflow::Module *module, const XML::Element& config):
    Filter(module, config) {}

  // Accept a frame
  virtual void accept(FramePtr frame) = 0;

  // Accept data
  void accept(DataPtr data) override
  {
    // Call down with type-checked data
    accept(data.check<Frame>());
  }
};

class FrameSink: public Sink
{
 public:
  // Construct with XML
  FrameSink(const Dataflow::Module *module, const XML::Element& config):
    Sink(module, config) {}

  // Accept a frame
  virtual void accept(FramePtr frame) = 0;

  // Accept data
  void accept(DataPtr data) override
  {
    // Call down with type-checked data
    accept(data.check<Frame>());
  }
};

//==========================================================================
}}} //namespaces

using namespace ViGraph::Module::Audio;

#endif // !__VIGRAPH_AUDIO_MODULE_H
