//==========================================================================
// ViGraph dataflow module: bitmap/vector-fill/vector-fill.cc
//
// Generate a bitmap from a filled vector
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../bitmap-module.h"
#include "../../vector/vector-module.h"
#include <cmath>

namespace {

//==========================================================================
// VectorFill
class VectorFill: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  VectorFill *create_clone() const override
  {
    return new VectorFill{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Input
  Input<Frame> input;
  Input<double> width;
  Input<double> height;

  // Output
  Output<Bitmap::Group> output;
};

//--------------------------------------------------------------------------
// Tick data
void VectorFill::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(nsamples, {}, tie(input, width, height), tie(output),
                 [&](const Frame& input, double width, double height,
                     Bitmap::Group& output)
  {
    Bitmap::Rectangle rect(width, height);
    rect.fill_polygons(input.points);
    output.add(rect);
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "vector-fill",
  "Vector fill",
  "bitmap",
  {},
  {
    { "input",  &VectorFill::input  },
    { "width",  &VectorFill::width  },
    { "height", &VectorFill::height }
  },
  {
    { "output", &VectorFill::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(VectorFill, module)
