//==========================================================================
// ViGraph dataflow module: time-series/plot/plot.cc
//
// Plot time series values as a vector frame
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../time-series-module.h"
#include "../../vector/vector-module.h"
#include <limits>

namespace {

const auto default_points{1000.0};

//==========================================================================
// Plot filter
class Plotter: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Plotter *create_clone() const override
  {
    return new Plotter{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Setting
  Setting<bool> auto_scale{true};

  // Input
  Input<Number> points{default_points};
  Input<DataCollection> input;

  // Output
  Output<Frame> output;
};

//--------------------------------------------------------------------------
// Tick data
void Plotter::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(input, points), tie(output),
                 [&](const DataCollection& input, Number points,
                     Frame& output)
  {
    if (!points) return;

    // Find min and max value across all datasets, if autoscaling
    // - otherwise just 0..1
    double max = auto_scale?numeric_limits<double>::min():1;
    double min = auto_scale?numeric_limits<double>::max():0;

    if (auto_scale)
    {
      for(auto& ds: input.datasets)
      {
        for(auto& s: ds.samples)
        {
          if (s.value > max) max = s.value;
          if (s.value < min) min = s.value;
        }
      }
    }

    // Find widest start and end across all datasets
    double start = numeric_limits<double>::max();
    double end   = numeric_limits<double>::min();

    for(auto& ds: input.datasets)
    {
      if (!ds.samples.size()) continue;
      double st = ds.samples.front().at;
      double en   = ds.samples.back().at;
      if (st < start) start = st;
      if (en > end) end = en;
    }

    if (end <= start) return;

    for(auto& ds: input.datasets)
    {
      if (!ds.samples.size()) continue;
      for(auto i=0; i<points; i++)
      {
        double frac = i/points;
        // Resample into points - note very slight rounding
        unsigned n = frac*ds.samples.size()+1e-12;
        if (n >= ds.samples.size()) n = ds.samples.size()-1;
        const auto& s = ds.samples[n];
        double x = -0.5 + (s.at-start)/(end-start);
        double y = -0.5 + (s.value-min)/((min==max)?1:max-min);

        // Clip
        if (y < -0.5 || y > 0.5) continue;

        // Add blanking on first point
        if (!i) output.points.push_back(Point(x, y));

        output.points.push_back(Point(x, y, Colour::white));
      }
    }
  });
}

Dataflow::SimpleModule module
{
  "plot",
  "Plot",
  "time-series",
  {
    { "auto",   &Plotter::auto_scale }
  },
  {
    { "points", &Plotter::points },
    { "input",  &Plotter::input  }
  },
  {
    { "output", &Plotter::output }
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Plotter, module)
