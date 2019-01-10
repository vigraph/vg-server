//==========================================================================
// ViGraph dataflow module:
//  laser/filters/add-vertex-repeats/add-vertex-repeats.cc
//
// Adds repeated points at sharp vertices
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../vector/vector-module.h"
#include "vg-laser.h"
#include <cmath>

namespace {

const double default_repeats = 3;
const double default_max_angle = 30;

//==========================================================================
// AddVertexRepeats filter
class AddVertexRepeatsFilter: public FrameFilter
{
  double max_angle;  // in degrees
  int repeats;
  Laser::Optimiser optimiser;

  // Filter/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void accept(FramePtr frame) override;

public:
  // Construct
  AddVertexRepeatsFilter(const Dataflow::Module *module,
                         const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
//  <add-vertex-repeats max-angle="30" repeats="3"/>
AddVertexRepeatsFilter::AddVertexRepeatsFilter(
                                         const Dataflow::Module *module,
                                         const XML::Element& config):
  Element(module, config), FrameFilter(module, config)
{
  repeats = config.get_attr_int("repeats", default_repeats);
  max_angle = config.get_attr_real("max-angle", default_max_angle);
}

//--------------------------------------------------------------------------
// Set a control property
void AddVertexRepeatsFilter::set_property(const string& property,
                                            const SetParams& sp)
{
  if (property == "repeats")
    update_prop_int(repeats, sp);
  else if (property == "max-angle")
    update_prop(max_angle, sp);
}

//--------------------------------------------------------------------------
// Process some data
void AddVertexRepeatsFilter::accept(FramePtr frame)
{
  frame->points = optimiser.add_vertex_repeats(frame->points,
                                               max_angle*pi/180, // to radians
                                               repeats);
  send(frame);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "add-vertex-repeats",
  "Add vertex repeats",
  "Add additional points at sharp vertices for laser scanners",
  "laser",
  {
    { "repeats", { "Number of points to add at a vertex",
          Value::Type::number, true } },
    { "max-angle", { "Maximum turn angle at a vertex before adding points",
          Value::Type::number, true } }
  },
  { "VectorFrame" }, // inputs
  { "VectorFrame" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(AddVertexRepeatsFilter, module)
