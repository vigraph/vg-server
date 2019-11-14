//==========================================================================
// ViGraph dataflow module: audio/position/position.cc
//
// Position module
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../audio-module.h"
#include "vg-geometry.h"

namespace {

using namespace ViGraph::Geometry;

//==========================================================================
// Position
class Position: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Position *create_clone() const override
  {
    return new Position{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Input<double> input{0.0};
  Input<double> x{0.0};
  Output<double> left;
  Output<double> right;
};

//--------------------------------------------------------------------------
// Tick data
void Position::tick(const TickData& td)
{
  const auto sample_rate = max(left.get_sample_rate(),
                               right.get_sample_rate());
  const auto nsamples = td.samples_in_tick(sample_rate);
  sample_iterate(nsamples, {}, tie(input, x), tie(left, right),
                 [](double i, double x, double& l, double& r)
  {
    x = min(max(x, -0.5), 0.5) + 0.5;
    l = i * cos(x * pi / 2);
    r = i * sin(x * pi / 2);
  });
}

Dataflow::SimpleModule module
{
  "position",
  "Position",
  "audio",
  {},
  {
    { "input",    &Position::input },
    { "x",        &Position::x }
  },
  {
    { "left",     &Position::left },
    { "right",    &Position::right }
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Position, module)
