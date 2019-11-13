//==========================================================================
// ViGraph dataflow module: core/polar-position/polar-position.cc
//
// Polar position module - creates Cartesian from polar co-ordinates
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../core-module.h"
#define _USE_MATH_DEFINES
#include <cmath>

namespace {

//==========================================================================
// Polar position filter
class PolarPositionFilter: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  PolarPositionFilter *create_clone() const override
  {
    return new PolarPositionFilter{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Input<double> angle{0.0}; // 0..1 full circle, positive anticlockwise, 0 = +x
  Input<double> distance{0.0};
  Output<double> x;
  Output<double> y;
};

//--------------------------------------------------------------------------
// Tick data
void PolarPositionFilter::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(max(x.get_sample_rate(),
                                               y.get_sample_rate()));
  sample_iterate(nsamples, {}, tie(angle, distance), tie(x, y),
                 [&](double angle, double distance, double& x, double& y)
  {
    auto rad = angle * 2 * M_PI;
    x = distance * cos(rad);
    y = distance * sin(rad);
  });
}

Dataflow::SimpleModule module
{
  "polar-position",
  "Polar Position",
  "core",
  {},
  {
    { "angle",    &PolarPositionFilter::angle    },
    { "distance", &PolarPositionFilter::distance }
  },
  {
    { "x", &PolarPositionFilter::x },
    { "y", &PolarPositionFilter::y }
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(PolarPositionFilter, module)
