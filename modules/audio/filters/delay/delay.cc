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
private:
  Time::Duration delay;
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
  double get_time() const { return delay.seconds(); }
  void set_time(double time);
  void on() { enabled = true; }
  void off() { enabled = false; }
};

//--------------------------------------------------------------------------
// Set delay time
void DelayFilter::set_time(double t)
{
  delay = Time::Duration{max(t, 0.0)};
  if (!buffer.empty())
  {
    const auto new_size = static_cast<uint64_t>(delay.seconds() * sample_rate);
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
void DelayFilter::accept(FragmentPtr fragment)
{
  if (enabled && delay)
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
    tick_sent = true;
  }

  send(fragment);
}

//--------------------------------------------------------------------------
// If nothing received but have buffered stuff, send it
void DelayFilter::post_tick(const TickData& td)
{
  if (enabled && delay && !tick_sent)
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
void DelayFilter::notify_target_of(const string& property)
{
  if (property == "enable")
    enabled = false;
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
    { "time", { "Time to delay for", Value::Type::number,
                { &DelayFilter::get_time, &DelayFilter::set_time }, true } },
    { "feedback", { "Amount of feedback (0-?)", Value::Type::number,
                    &DelayFilter::feedback, true } },
    { "enable", { "Enable the filter", Value::Type::trigger,
                  &DelayFilter::on, true } },
    { "disable", { "Disable the filter", Value::Type::trigger,
                   &DelayFilter::off, true } },
  },
  { "Audio" }, // inputs
  { "Audio" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(DelayFilter, module)
