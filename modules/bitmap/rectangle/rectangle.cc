//==========================================================================
// ViGraph dataflow module: bitmap/rectangle/rectangle.cc
//
// Bitmap rectangle source
//
// Copyright (c) 2018-2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../bitmap-module.h"

namespace {

//==========================================================================
// Rectangle source
class RectangleSource: public SimpleElement
{
private:
  // Source/Element virtuals
  void tick(const TickData& td) override;

  // Clone
  RectangleSource *create_clone() const override
  {
    return new RectangleSource{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Inputs
  Input<Number> width{1};
  Input<Number> height{1};

  // Output
  Output<Bitmap::Group> output;
};

//--------------------------------------------------------------------------
// Generate a bitmap
void RectangleSource::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(width, height), tie(output),
                 [&](Number width, Number height,
                     Bitmap::Group& output)
  {
    Bitmap::Rectangle rect(width, height);
    rect.fill(Colour::white);
    output.add(rect);
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "rectangle",
  "Rectangular bitmap",
  "bitmap",
  {},
  {
    { "width",     &RectangleSource::width     },
    { "height",    &RectangleSource::height    }
  },
  {
    { "output",    &RectangleSource::output    }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(RectangleSource, module)
