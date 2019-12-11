//==========================================================================
// ViGraph dataflow module: core/filter/filter.cc
//
// Basic high/low/band-pass filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../core-module.h"

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

  vector<double> buffer;

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Input<Mode> mode{Mode::low_pass};
  Input<double> input{0.0};
  Input<double> cutoff{1.0};
  Input<double> resonance{1.0};
  Input<double> steepness{1.0};
  Output<double> output;
};

//--------------------------------------------------------------------------
// Generate a fragment
void Filter::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(input, mode, cutoff, resonance, steepness),
                 tie(output),
                 [&](double input, Mode mode, double cutoff,
                     double resonance, double steepness, double& o)
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
      if (!b)
        buffer[b] += cutoff * (input - buffer[b] +
                               feedback * (buffer[b] - buffer[b + 1]));
      else
        buffer[b] += cutoff * (buffer[b - 1] - buffer[b]);
    }
    switch (mode)
    {
      case Mode::low_pass:
        o = buffer.back();
        break;
      case Mode::high_pass:
        o = input - buffer.back();
        break;
      case Mode::band_pass:
        o = buffer.front() - buffer.back();
        break;
    }
  });
}

Dataflow::SimpleModule module
{
  "filter",
  "Filter",
  "core",
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
