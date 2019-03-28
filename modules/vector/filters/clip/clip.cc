//==========================================================================
// ViGraph dataflow module: vector/filters/clip/clip.cc
//
// Clip filter
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module.h"

namespace {

using Colour::RGB;

//==========================================================================
// Clip filter
class ClipFilter: public FrameFilter
{
public:
  // Bounding cube
  double min_x = -0.5;
  double min_y = -0.5;
  double min_z = -0.5;
  double max_x = 0.5;
  double max_y = 0.5;
  double max_z = 0.5;

  bool exclude{false};    // Inverse, clip out a rectangle
  Colour::intens_t alpha{0.0}; // Fade
  RGB outline_colour;

private:
  // Filter/Element virtuals
  void accept(FramePtr frame) override;

public:
  using FrameFilter::FrameFilter;

  // Getters/Setters
  string get_outline() const { return outline_colour.str(); }
  void set_outline(const string& colour) { outline_colour = RGB{colour}; }
};

//--------------------------------------------------------------------------
// Process some data
void ClipFilter::accept(FramePtr frame)
{
  // First (bad) attempt - blank all points inside/outside clip,
  // and the following one
  bool last_was_blanked{false};
  Point last_unclipped_point;
  for(auto& p: frame->points)
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

  // Add outline (2D) for testing if required
  if (!outline_colour.is_black())
  {
    frame->points.push_back(Point(min_x, min_y, min_z));
    frame->points.push_back(Point(min_x, max_y, outline_colour));
    frame->points.push_back(Point(max_x, max_y, max_z, outline_colour));
    frame->points.push_back(Point(max_x, min_y, outline_colour));
    frame->points.push_back(Point(min_x, min_y, min_z, outline_colour));
  }

  send(frame);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "clip",
  "Clip",
  "Clip to a rectangular region",
  "vector",
  {
    { "min.x", { "Minimum X value", Value::Type::number,
                 &ClipFilter::min_x, true } },
    { "min.y", { "Minimum Y value", Value::Type::number,
                 &ClipFilter::min_y, true } },
    { "min.z", { "Minimum Z value", Value::Type::number,
                 &ClipFilter::min_z, true } },
    { "max.x", { "Maximum X value", Value::Type::number,
                 &ClipFilter::max_x, true } },
    { "max.y", { "Maximum Y value", Value::Type::number,
                 &ClipFilter::max_y, true } },
    { "max.z", { "Maximum Z value", Value::Type::number,
                 &ClipFilter::max_z, true } },
    { "exclude", { "Whether to remove points inside the box",
                   Value::Type::boolean,
                   &ClipFilter::exclude, true } },
    { "alpha", { "Alpha level to apply to clipped points", Value::Type::number,
                 &ClipFilter::alpha, true } },
    { "Outline", { "Outline colour", Value::Type::text,
                   { &ClipFilter::get_outline, &ClipFilter::set_outline },
                   true } },
  },
  { "VectorFrame" }, // inputs
  { "VectorFrame" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ClipFilter, module)
