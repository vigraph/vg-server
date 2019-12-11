//==========================================================================
// ViGraph dataflow module: bitmap/tile/tile.cc
//
// Tile filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../bitmap-module.h"
#include <cmath>

namespace {

//==========================================================================
// Tile
class Tile: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Tile *create_clone() const override
  {
    return new Tile{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Input<double> x{1.0};
  Input<double> y{1.0};

  // Input
  Input<Bitmap::Group> input;

  // Output
  Output<Bitmap::Group> output;
};

//--------------------------------------------------------------------------
// Tick data
void Tile::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(x, y, input), tie(output),
                 [&](double x, double y, const Bitmap::Group& input,
                     Bitmap::Group& output)
  {
    for(auto& item: input.items)
    {
      auto width = item.rect.get_width();
      auto height = item.rect.get_height();
      Bitmap::Rectangle tiled(width*x, height*y);
      for(auto i=0; i<x; i++)
        for(auto j=0; j<y; j++)
          item.rect.blit(Vector(i*width, j*height), tiled);

      output.add(tiled);
    }
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "tile",
  "Bitmap tile",
  "bitmap",
  {},
  {
    { "x",     &Tile::x     },
    { "y",     &Tile::y     },
    { "input", &Tile::input }
  },
  {
    { "output", &Tile::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Tile, module)
