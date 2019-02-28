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
  double feedback = 0.5;

  struct Buffer
  {
    unsigned pos = 0;
    vector<sample_t> samples;
  };
  map<Speaker, Buffer> buffer; // Buffer per speaker

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
  feedback = config.get_attr_real("feedback", 0.5);
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
      for (auto& bit: buffer)
      {
        auto& b = bit.second;
        auto tmp = vector<sample_t>(max(new_size, b.samples.size()));
        auto it = tmp.begin();
        if (new_size > b.samples.size())
          it += (new_size - b.samples.size());
        else
        {
          b.pos += (b.samples.size() - new_size);
          while (b.pos >= b.samples.size())
            b.pos -= b.samples.size();
        }
        auto bpos = b.samples.begin() + b.pos;
        it = copy(bpos, b.samples.end(), it);
        copy(b.samples.begin(), bpos, it);
        tmp.resize(new_size);
        b.samples = tmp;
        b.pos = 0;
      }
    }
  }
  else if (property == "feedback")
    feedback = sp.v.d;
}

//--------------------------------------------------------------------------
// Process some data
void DelayFilter::accept(FragmentPtr fragment)
{
  if (delay)
  {
    for (auto& wit: fragment->waveforms)
    {
      const auto c = wit.first;
      auto& w = wit.second;
      auto bit = buffer.find(c);
      if (bit == buffer.end())
      {
        bit = buffer.emplace(c, Buffer{}).first;
        bit->second.samples.resize(delay.seconds() * sample_rate);
      }
      auto& buf = bit->second;
      for (auto i = 0u; i < w.size(); ++i)
      {
        auto& s = w[i];
        auto& b = buf.samples[buf.pos];
        s += feedback * b;
        b = s;
        if (++buf.pos >= buf.samples.size())
          buf.pos = 0;
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
    { "feedback", { {"Amount of feedback (0-?)", "0.5"}, Value::Type::number,
                                                         "@feedback", true } },
  },
  { "Audio" }, // inputs
  { "Audio" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(DelayFilter, module)
