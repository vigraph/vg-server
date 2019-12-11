//==========================================================================
// ViGraph dataflow module: vector/rotate/rotate.cc
//
// Rotate filter
//
// Copyright (c) 2017-2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../vector-module.h"
#include <cmath>

namespace {

//==========================================================================
// Rotate
class Rotate: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Rotate *create_clone() const override
  {
    return new Rotate{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Input<Number> x{0.0};      // 0..1 - 1 = 360 deg = 2pi rad
  Input<Number> y{0.0};
  Input<Number> z{0.0};

  // Input
  Input<Frame> input;

  // Output
  Output<Frame> output;
};

//--------------------------------------------------------------------------
// Tick data
void Rotate::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(x, y, z, input), tie(output),
                 [&](double x, double y, double z, const Frame& input,
                     Frame& output)
  {
    // Precalculate useful stuff
    // ! Could optimise only on changes?
    double sinx = sin(x*2*pi);
    double cosx = cos(x*2*pi);
    double siny = sin(y*2*pi);
    double cosy = cos(y*2*pi);
    double sinz = sin(z*2*pi);
    double cosz = cos(z*2*pi);

    // Modify all points in the frame
    output = input;
    for(auto& p: output.points)
    {
      double xy = cosx*p.y - sinx*p.z;
      double xz = sinx*p.y + cosx*p.z;
      double yz = cosy*xz  - siny*p.x;
      double yx = siny*xz  + cosy*p.x;
      double zx = cosz*yx  - sinz*xy;
      double zy = sinz*yx  + cosz*xy;

      p.x = zx;
      p.y = zy;
      p.z = yz;
    }
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "rotate",
  "Rotate",
  "vector",
  {},
  {
    { "x",     &Rotate::x     },
    { "y",     &Rotate::y     },
    { "z",     &Rotate::z     },
    { "input", &Rotate::input }
  },
  {
    { "output", &Rotate::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Rotate, module)
