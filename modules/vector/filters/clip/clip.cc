//==========================================================================
// ViGraph dataflow module: vector/filters/clip/clip.cc
//
// Clip filter
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module.h"

namespace {

//==========================================================================
// Clip filter
class ClipFilter: public FrameFilter
{
  Point min, max;         // Bounding cube
  bool exclude{false};    // Inverse, clip out a rectangle
  Colour::intens_t alpha{0.0}; // Fade
  Colour::RGB outline_colour;

  // Filter/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void accept(FramePtr frame) override;

public:
  // Construct
  ClipFilter(const Dataflow::Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
//  <clip exclude="false" alpha="0" outline="black">
//    <min x="-0.5" y="-0.5" z="-0.5"/>
//    <max x="0.5"  y="0.5"  z="0.5"/>
// </clip>
ClipFilter::ClipFilter(const Dataflow::Module *module,
                       const XML::Element& config):
  FrameFilter(module, config)
{
  const XML::Element& min_e = config.get_child("min");
  min.x = min_e.get_attr_real("x", -0.5);
  min.y = min_e.get_attr_real("y", -0.5);
  min.z = min_e.get_attr_real("z", -0.5);

  const XML::Element& max_e = config.get_child("max");
  max.x = max_e.get_attr_real("x", 0.5);
  max.y = max_e.get_attr_real("y", 0.5);
  max.z = max_e.get_attr_real("z", 0.5);

  exclude = config.get_attr_bool("exclude");
  alpha = config.get_attr_real("alpha");
  if (config.has_attr("outline"))
    outline_colour = Colour::RGB(config["outline"]);
}

//--------------------------------------------------------------------------
// Set a control property
void ClipFilter::set_property(const string& property, const SetParams& sp)
{
       if (property == "min.x") update_prop(min.x, sp);
  else if (property == "min.y") update_prop(min.y, sp);
  else if (property == "min.z") update_prop(min.z, sp);
  else if (property == "max.x") update_prop(max.x, sp);
  else if (property == "max.y") update_prop(max.y, sp);
  else if (property == "max.z") update_prop(max.z, sp);
  else if (property == "alpha") update_prop(alpha, sp);
}

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

    bool outside_bb = p.x < min.x || p.y < min.y || p.z < min.z
                   || p.x > max.x || p.y > max.y || p.z > max.z;
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
    frame->points.push_back(min);
    frame->points.push_back(Point(min.x, max.y, outline_colour));
    frame->points.push_back(Point(max, outline_colour));
    frame->points.push_back(Point(max.x, min.y, outline_colour));
    frame->points.push_back(Point(min, outline_colour));
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
    { "min.x", { "Minimum X value", Value::Type::number, "min/@x", true } },
    { "min.y", { "Minimum Y value", Value::Type::number, "min/@y", true } },
    { "min.z", { "Minimum Z value", Value::Type::number, "min/@z", true } },
    { "max.x", { "Maximum X value", Value::Type::number, "max/@x", true } },
    { "max.y", { "Maximum Y value", Value::Type::number, "max/@y", true } },
    { "max.z", { "Maximum Z value", Value::Type::number, "max/@z", true } },
    { "exclude", { "Whether to remove points inside the box",
          Value::Type::boolean, "@exclude" } },
    { "alpha", { "Alpha level to apply to clipped points",
          Value::Type::number, "@alpha", true } }
  },
  { "VectorFrame" }, // inputs
  { "VectorFrame" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ClipFilter, module)
