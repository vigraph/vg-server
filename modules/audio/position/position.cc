//==========================================================================
// ViGraph dataflow module: audio/position/position.cc
//
// Position module - mono in, stereo out
// !TODO - generalise
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
  Input<AudioData> input;
  Input<Number> x{0.0};
  Output<AudioData> output;
};

//--------------------------------------------------------------------------
// Tick data
void Position::tick(const TickData& td)
{
  const auto sample_rate = output.get_sample_rate();
  const auto nsamples = td.samples_in_tick(sample_rate);
  sample_iterate(td, nsamples, {}, tie(input, x), tie(output),
                 [](const AudioData& i, Number x,
                    AudioData& o)
  {
    x = min(max(x, -0.5), 0.5) + 0.5;
    o.nchannels = 2;
    o.channels[0] = i.channels[0] * cos(x * pi / 2);
    o.channels[1] = i.channels[0] * sin(x * pi / 2);
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
    { "output",   &Position::output }
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Position, module)
