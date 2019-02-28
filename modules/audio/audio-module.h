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
// Speaker enum
enum class Speaker
{
  front_left,
  front_right,
  front_center,
  low_frequency,
  back_left,
  back_right,
  front_left_of_center,
  front_right_of_center,
  back_center,
  side_left,
  side_right,
  left_height,
  right_height,
};

//==========================================================================
// Channels to speaker mapping
const auto channel_mappings = map<unsigned, vector<Speaker>>
{
  {1, {Speaker::front_center}},
  {2, {Speaker::front_left,
       Speaker::front_right}},
  {3, {Speaker::front_left,
       Speaker::front_right,
       Speaker::front_center}},
  {4, {Speaker::front_left,
       Speaker::front_right,
       Speaker::back_left,
       Speaker::back_right}},
  {5, {Speaker::front_left,
       Speaker::front_right,
       Speaker::front_center,
       Speaker::back_left,
       Speaker::back_right}},
  {6, {Speaker::front_left,
       Speaker::front_right,
       Speaker::front_center,
       Speaker::low_frequency,
       Speaker::back_left,
       Speaker::back_right}},
  {7, {Speaker::front_left,
       Speaker::front_right,
       Speaker::front_center,
       Speaker::back_left,
       Speaker::back_right,
       Speaker::front_left_of_center,
       Speaker::front_right_of_center}},
  {8, {Speaker::front_left,
       Speaker::front_right,
       Speaker::front_center,
       Speaker::low_frequency,
       Speaker::back_left,
       Speaker::back_right,
       Speaker::front_left_of_center,
       Speaker::front_right_of_center}},
};

//==========================================================================
// Audio fragment - section of waveform across (maybe) multiple channels
struct Fragment: public Data
{
  timestamp_t timestamp;
  map<Speaker, vector<sample_t>> waveforms;

  Fragment(timestamp_t t):
    timestamp(t) {}
};

using FragmentPtr = shared_ptr<Fragment>;

//==========================================================================
// Specialisations of Dataflow classes for Fragment data
class FragmentFilter: public Filter
{
 public:
  // Construct with XML
  FragmentFilter(const Dataflow::Module *module, const XML::Element& config):
    Filter(module, config) {}

  // Accept a frame
  virtual void accept(FragmentPtr frame) = 0;

  // Accept data
  void accept(DataPtr data) override
  {
    // Call down with type-checked data
    accept(data.check<Fragment>());
  }
};

class FragmentSink: public Sink
{
 public:
  // Construct with XML
  FragmentSink(const Dataflow::Module *module, const XML::Element& config):
    Sink(module, config) {}

  // Accept a frame
  virtual void accept(FragmentPtr frame) = 0;

  // Accept data
  void accept(DataPtr data) override
  {
    // Call down with type-checked data
    accept(data.check<Fragment>());
  }
};

//==========================================================================
}}} //namespaces

using namespace ViGraph::Module::Audio;

#endif // !__VIGRAPH_AUDIO_MODULE_H
