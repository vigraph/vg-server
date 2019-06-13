//==========================================================================
// ViGraph dataflow module:
//    audio/filters/reverb/reverb.cc
//
// Audio reverb filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../audio-module.h"

namespace {

using namespace ViGraph::Dataflow;

//==========================================================================
// Reverb filter
class ReverbFilter: public FragmentFilter
{
private:
  Time::Duration reverb;
  struct Buffer
  {
    unsigned pos = 0;
    vector<sample_t> samples;
  };
  map<Speaker, Buffer> buffer; // Buffer per speaker
  bool enabled = true;

  bool tick_sent = false;

  // Source/Element virtuals
  void accept(FragmentPtr fragment) override;
  void pre_tick(const TickData&) override { tick_sent = false; }
  void post_tick(const TickData& td) override;
  void notify_target_of(const string& property) override;

public:
  double feedback = 0.5;

  using FragmentFilter::FragmentFilter;

  // Getters/Setters
  double get_time() const { return reverb.seconds(); }
  void set_time(double time);
  void on() { enabled = true; }
  void off() { enabled = false; }
};

//--------------------------------------------------------------------------
// Set reverb time
void ReverbFilter::set_time(double t)
{
  reverb = Time::Duration{max(t, 0.0)};
  if (!buffer.empty())
  {
    const auto new_size = static_cast<uint64_t>(reverb.seconds() * sample_rate);
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

//--------------------------------------------------------------------------
// Process some data
void ReverbFilter::accept(FragmentPtr fragment)
{
  if (enabled && reverb)
  {
    for (auto& wit: fragment->waveforms)
    {
      const auto c = wit.first;
      auto& w = wit.second;
      auto bit = buffer.find(c);
      if (bit == buffer.end())
      {
        bit = buffer.emplace(c, Buffer{}).first;
        bit->second.samples.resize(reverb.seconds() * sample_rate);
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
    tick_sent = true;
  }

  send(fragment);
}

//--------------------------------------------------------------------------
// If nothing received but have buffered stuff, send it
void ReverbFilter::post_tick(const TickData& td)
{
  if (enabled && reverb && !tick_sent)
  {
    const auto nsamples = td.samples();

    // Make an empty fragment of the correct channels and size
    auto fragment = FragmentPtr(new Fragment(td.t));
    for (const auto& b: buffer)
      fragment->waveforms[b.first].resize(nsamples);

    // Run the empty fragment through the standard process
    accept(fragment);
  }
}

//--------------------------------------------------------------------------
// If recipient of triggers default to disabled
void ReverbFilter::notify_target_of(const string& property)
{
  if (property == "enable")
    enabled = false;
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "reverb",
  "Audio reverb",
  "Audio reverb",
  "audio",
  {
    { "time", { "Time to reverb for", Value::Type::number,
                { &ReverbFilter::get_time, &ReverbFilter::set_time }, true } },
    { "feedback", { "Amount of feedback (0-?)", Value::Type::number,
                    &ReverbFilter::feedback, true } },
    { "enable", { "Enable the filter", Value::Type::trigger,
                  &ReverbFilter::on, true } },
    { "disable", { "Disable the filter", Value::Type::trigger,
                   &ReverbFilter::off, true } },
  },
  { "Audio" }, // inputs
  { "Audio" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ReverbFilter, module)
