//==========================================================================
// ViGraph dataflow module: vector/oscilloscope/oscilloscope.cc
//
// Oscilloscope filter
//
// Copyright (c) 2017-2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../vector-module.h"
#include <cmath>

namespace {

const auto default_sample_rate = 44100;

//--------------------------------------------------------------------------
// Slope
enum class Slope
{
  free,
  rising,
  falling
};

}

namespace ViGraph { namespace Dataflow {

template<> inline
string get_module_type<Slope>() { return "oscilloscope-slope"; }

template<> inline void set_from_json(Slope& slope,
                                     const JSON::Value& json)
{
  const auto& s = json.as_str();

  if (s == "free")
    slope = Slope::free;
  else if (s == "rising")
    slope = Slope::rising;
  else if (s == "falling")
    slope = Slope::falling;
  else
    slope = Slope::free;
}

template<> inline JSON::Value get_as_json(const Slope& slope)
{
  switch (slope)
  {
    case Slope::free:
      return "free";
    case Slope::rising:
      return "rising";
    case Slope::falling:
      return "falling";
  }
}

}} // namespaces

namespace {

//==========================================================================
// Oscilloscope
class Oscilloscope: public SimpleElement
{
private:
  vector<double> buffer;

  // Element virtuals
  void setup(const SetupContext& context) override;
  void update_sample_rate() override {}
  void tick(const TickData& td) override;

  // Clone
  Oscilloscope *create_clone() const override
  {
    return new Oscilloscope{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Setting<double> sample_rate{default_sample_rate};
  Input<Slope> slope{Slope::free};
  Input<double> max_points{0};
  Input<double> level{0};

  // Input
  Input<double> input;

  // Output
  Output<Frame> output;
};

//--------------------------------------------------------------------------
// Setup
void Oscilloscope::setup(const SetupContext&)
{
  slope.set_sample_rate(sample_rate);
  input.set_sample_rate(sample_rate);
}

//--------------------------------------------------------------------------
// Tick data
void Oscilloscope::tick(const TickData& td)
{
  const auto& in = input.get_buffer();
  buffer.insert(buffer.end(), in.begin(), in.end());

  const auto output_rate = output.get_sample_rate();
  const auto out_samples = td.samples_in_tick(output_rate);
  auto out = output.get_buffer();
  if (out_samples)
  {
    const auto s = slope.get();
    const auto max = max_points.get();
    const auto lev = level.get();
    auto i = 0u;
    for (auto o = 0u; o < out_samples; ++o)
    {
      out.data.emplace_back();
      auto& out_data = out.data[o];
      const auto iend = (o + 1) * buffer.size() / out_samples;
      auto added = 0;
      auto started = false;
      auto istart = i;

      for (; i < iend && (!max || added <= max); ++i)
      {
        bool add = started;
        switch (s)
        {
          case Slope::free:
            add = true;
            break;
          case Slope::rising:
            if (!started)
            {
              if (i && buffer[i - 1] < lev && buffer[i] >= lev)
              {
                started = true;
                istart = i;
                add = true;
              }
            }
            break;
          case Slope::falling:
            if (!started)
            {
              if (i && buffer[i - 1] > lev && buffer[i] <= lev)
              {
                started = true;
                istart = i;
                add = true;
              }
            }
            break;
        }
        if (add)
        {
          const auto x = static_cast<double>(i - istart)
                         / (max ? max : (iend - istart)) - 0.5;
          out_data.points.emplace_back(x, buffer[i] * 0.5, 0, Colour::white);
          ++added;
        }
      }
    }
    buffer.clear();
  }
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "oscilloscope",
  "Oscilloscope",
  "vector",
  {
    { "sample-rate",  &Oscilloscope::sample_rate },
  },
  {
    { "slope",        &Oscilloscope::slope },
    { "max-points",   &Oscilloscope::max_points },
    { "level",        &Oscilloscope::level },
    { "input",        &Oscilloscope::input },
  },
  {
    { "output",       &Oscilloscope::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Oscilloscope, module)
