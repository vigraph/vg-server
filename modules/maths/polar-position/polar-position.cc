//==========================================================================
// ViGraph dataflow module: maths/polar-position/polar-position.cc
//
// Polar position module - creates Cartesian from polar co-ordinates
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#define _USE_MATH_DEFINES
#include "../maths-module.h"

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
  Input<Number> angle{0.0}; // 0..1 full circle, positive anticlockwise, 0 = +x
  Input<Number> distance{0.0};
  Output<Number> x;
  Output<Number> y;
};

//--------------------------------------------------------------------------
// Tick data
void PolarPositionFilter::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(max(x.get_sample_rate(),
                                               y.get_sample_rate()));
  sample_iterate(td, nsamples, {}, tie(angle, distance), tie(x, y),
                 [&](Number angle, Number distance, Number& x, Number& y)
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
  "maths",
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
