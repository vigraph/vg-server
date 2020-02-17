//==========================================================================
// ViGraph dataflow module: audio/filter/filter.cc
//
// Basic high/low/band-pass audio filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../audio-module.h"

namespace {

//--------------------------------------------------------------------------
// Filter mode
enum class Mode
{
  low_pass,
  high_pass,
  band_pass
};

}

namespace ViGraph { namespace Dataflow {

template<> inline
string get_module_type<Mode>() { return "filter-mode"; }

template<> inline void set_from_json(Mode& mode,
                                     const JSON::Value& json)
{
  const auto& m = json.as_str();

  if (m == "low-pass")
    mode = Mode::low_pass;
  else if (m == "high-pass")
    mode = Mode::high_pass;
  else if (m == "band-pass")
    mode = Mode::band_pass;
  else
    mode = Mode::low_pass;
}

template<> inline JSON::Value get_as_json(const Mode& mode)
{
  switch (mode)
  {
    case Mode::low_pass:
      return "low-pass";
    case Mode::high_pass:
      return "high-pass";
    case Mode::band_pass:
      return "band-pass";
  }
  return {};
}

}} // namespaces

namespace {

//==========================================================================
// Filter
class Filter: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Filter *create_clone() const override
  {
    return new Filter{module};
  }

  vector<AudioData> buffer;

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Input<Mode> mode{Mode::low_pass};
  Input<AudioData> input{0.0};
  Input<Number> cutoff{1.0};
  Input<Number> resonance{0.0};
  Input<Number> steepness{1.0};
  Output<AudioData> output;
};

//--------------------------------------------------------------------------
// Generate a fragment
void Filter::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(input, mode, cutoff, resonance, steepness),
                 tie(output),
                 [&](const AudioData& input, Mode mode, Number cutoff,
                     Number resonance, Number steepness, AudioData& o)
  {
    const auto stp = static_cast<unsigned>(max(1.0, steepness) + 1);
    if (stp != buffer.size())
      buffer.resize(stp);

    cutoff = min(max(cutoff, 0.0), 1.0);
    resonance = min(max(resonance, 0.0), 1.0);
    const auto feedback = resonance +
                          resonance / (1.0 - min(max(cutoff, 0.0), 0.9999));

    for (auto b = 0u; b < buffer.size(); ++b)
    {
      for(auto c=0; c<input.nchannels; c++)
      {
        auto& bc = buffer[b].channels[c];

        if (!b)
          bc += cutoff * (input.channels[c] - bc +
                          feedback * (bc - buffer[b + 1].channels[c]));
        else
          bc += cutoff * (buffer[b - 1].channels[c] - bc);
      }
    }

    switch (mode)
    {
      case Mode::low_pass:
        o.nchannels = input.nchannels;
        o.channels = buffer.back().channels;
        break;

      case Mode::high_pass:
      {
        o = input;
        const auto& bb = buffer.back();
        for(auto c=0; c<input.nchannels; c++)
          o.channels[c] -= bb.channels[c];
        break;
      }

      case Mode::band_pass:
      {
        o.nchannels = input.nchannels;
        const auto& bf = buffer.front();
        const auto& bb = buffer.back();
        for(auto c=0; c<input.nchannels; c++)
          o.channels[c] = bf.channels[c] - bb.channels[c];
        break;
      }
    }
  });
}

Dataflow::SimpleModule module
{
  "filter",
  "Filter",
  "audio",
  {},
  {
    { "input",      &Filter::input },
    { "mode",       &Filter::mode },
    { "cutoff",     &Filter::cutoff },
    { "resonance",  &Filter::resonance },
    { "steepness",  &Filter::steepness },
  },
  {
    { "output",     &Filter::output }
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Filter, module)
