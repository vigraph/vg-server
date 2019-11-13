//==========================================================================
// ViGraph dataflow module: vector/clip/clip.cc
//
// Clip filter
//
// Copyright (c) 2017-2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../vector-module.h"
#include <cmath>

namespace {

//==========================================================================
// Clip
class Clip: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Clip *create_clone() const override
  {
    return new Clip{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Settings
  Setting<bool> exclude{false};

  // Configuration
  Input<double> min_x{-0.5};
  Input<double> min_y{-0.5};
  Input<double> min_z{-0.5};
  Input<double> max_x{0.5};
  Input<double> max_y{0.5};
  Input<double> max_z{0.5};
  Input<double> alpha{0};

  // Input
  Input<Frame> input;

  // Output
  Output<Frame> output;
};

//--------------------------------------------------------------------------
// Tick data
void Clip::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(nsamples,
                 tie(exclude),
                 tie(min_x, min_y, min_z, max_x, max_y, max_z, alpha, input),
                 tie(output),
                 [&](bool exclude,
                     double min_x, double min_y, double min_z,
                     double max_x, double max_y, double max_z, double alpha,
                     const Frame& input,
                     Frame& output)
  {
    // First (bad) attempt - blank all points inside/outside clip,
    // and the following one
    bool last_was_blanked{false};
    Point last_unclipped_point;

    output = input;
    for(auto& p: output.points)
    {
      // If last one was blanked, blank this too to avoid a line from an
      // invalid point
      if (last_was_blanked) p.blank();

      bool outside_bb = p.x < min_x || p.y < min_y || p.z < min_z
                     || p.x > max_x || p.y > max_y || p.z > max_z;

      // Not wanted?
      if (exclude?!outside_bb:outside_bb)
      {
        // If alpha, leave it in place but fade it
        if (alpha > 0.0)
        {
          p.c.fade(alpha);
        }
        else
        {
          // Blank and shift to last good one (doesn't matter where it is, but
          // saves a scanner move)
          p = last_unclipped_point;
          p.blank();
          last_was_blanked = true;
        }
      }
      else
      {
        last_was_blanked = false;
        last_unclipped_point = p;
      }
    }
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "clip",
  "Clip",
  "vector",
  {
    { "exclude", &Clip::exclude }
  },
  {
    { "min-x", &Clip::min_x },
    { "min-y", &Clip::min_y },
    { "min-z", &Clip::min_z },
    { "max-x", &Clip::max_x },
    { "max-y", &Clip::max_y },
    { "max-z", &Clip::max_z },
    { "alpha", &Clip::alpha },
    { "input", &Clip::input }
  },
  {
    { "output", &Clip::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Clip, module)
