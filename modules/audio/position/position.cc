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
  Input<Number> input{0.0};
  Input<Number> x{0.0};
  Output<Number> left;
  Output<Number> right;
};

//--------------------------------------------------------------------------
// Tick data
void Position::tick(const TickData& td)
{
  const auto sample_rate = max(left.get_sample_rate(),
                               right.get_sample_rate());
  const auto nsamples = td.samples_in_tick(sample_rate);
  sample_iterate(td, nsamples, {}, tie(input, x), tie(left, right),
                 [](Number i, Number x, Number& l, Number& r)
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
