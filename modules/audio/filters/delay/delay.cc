//==========================================================================
// ViGraph dataflow module:
//    audio/filters/delay/delay.cc
//
// Audio delay filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../audio-module.h"

namespace {

using namespace ViGraph::Dataflow;

//==========================================================================
// Delay filter
class DelayFilter: public FragmentFilter
{
  Time::Duration delay;
  struct Buffer
  {
    unsigned pos = 0;
    vector<sample_t> samples;
  };
  vector<Buffer> buffer; // Buffer per channel

  // Source/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void accept(FragmentPtr fragment) override;

public:
  DelayFilter(const Dataflow::Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML:
//   <delay time="0.125" />
DelayFilter::DelayFilter(const Dataflow::Module *module,
                         const XML::Element& config):
    Element(module, config), FragmentFilter(module, config)
{
  delay = Time::Duration{config.get_attr_real("time", 0)};
}

//--------------------------------------------------------------------------
// Set a control property
void DelayFilter::set_property(const string& property, const SetParams& sp)
{
  if (property == "time")
  {
    delay = Time::Duration{max(sp.v.d, 0.0)};
    if (!buffer.empty())
    {
      const auto new_size = static_cast<unsigned long>(delay.seconds()
                                                       * sample_rate);
      for (auto& b: buffer)
      {
        auto tmp = vector<sample_t>(max(new_size, b.samples.size()));
        auto it = tmp.begin();
        if (new_size > b.samples.size())
          it += (new_size - b.samples.size());
        auto bpos = b.samples.begin() + b.pos;
        it = copy(bpos, b.samples.end(), it);
        copy(b.samples.begin(), bpos, it);
        tmp.resize(new_size);
        b.samples = tmp;
        b.pos = 0;
      }
    }
  }
}

//--------------------------------------------------------------------------
// Process some data
void DelayFilter::accept(FragmentPtr fragment)
{
  if (delay)
  {
    if (fragment->nchannels > buffer.size())
    {
      buffer.resize(fragment->nchannels);
      for (auto& b: buffer)
        b.samples.resize(delay.seconds() * sample_rate);
    }

    for (auto i = 0u; i < fragment->waveform.size() / fragment->nchannels; ++i)
    {
      for (auto c = 0u; c < fragment->nchannels; ++c)
      {
        auto& s = fragment->waveform[i * fragment->nchannels + c];
        auto& b = buffer[c].samples[buffer[c].pos];
        sample_t t = s;
        s = b;
        b = t;
        if (++buffer[c].pos >= buffer[c].samples.size())
          buffer[c].pos = 0;
      }
    }
  }

  send(fragment);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "delay",
  "Audio delay",
  "Audio delay",
  "audio",
  {
    { "time", { {"Time to delay for", "0"}, Value::Type::number,
                                            "@time", true } },
  },
  { "Audio" }, // inputs
  { "Audio" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(DelayFilter, module)
