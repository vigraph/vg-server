//==========================================================================
// ViGraph dataflow module: laser/filters/optimise/optimise.cc
//
// Point / blanking / path optimiser
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../vector/vector-module.h"
#include "vg-laser.h"
#include <cmath>

namespace {

//==========================================================================
// Optimise filter
class OptimiseFilter: public FrameFilter
{
  coord_t max_distance;
  coord_t max_angle;     // in rad
  int vertex_repeats;
  Laser::Optimiser optimiser;

  // Filter/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void accept(FramePtr frame) override;

public:
  // Construct
  OptimiseFilter(const Dataflow::Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
//  <optimise max-distance="0.001"/>
OptimiseFilter::OptimiseFilter(const Dataflow::Module *module,
                               const XML::Element& config):
  Element(module, config), FrameFilter(module, config)
{
  max_distance = config.get_attr_real("max-distance");
  optimiser.enable_infill(max_distance);

  max_angle = config.get_attr_real("max-angle")*pi/180;
  vertex_repeats = config.get_attr_int("vertex-repeats");
  optimiser.enable_vertex_repeats(max_angle, vertex_repeats);
}

//--------------------------------------------------------------------------
// Set a control property
void OptimiseFilter::set_property(const string& property, const SetParams& sp)
{
  if (property == "max-distance")
  {
    update_prop(max_distance, sp);
    optimiser.enable_infill(max_distance);
  }
  else if (property == "max-angle")
  {
    update_prop(max_angle, sp);
    optimiser.enable_vertex_repeats(max_angle, vertex_repeats);
  }
  else if (property == "vertex-repeats")
  {
    update_prop_int(vertex_repeats, sp);
    optimiser.enable_vertex_repeats(max_angle, vertex_repeats);
  }
}

//--------------------------------------------------------------------------
// Process some data
void OptimiseFilter::accept(FramePtr frame)
{
  // Fast null operation
  if (!max_distance && !max_angle)
  {
    send(frame);
    return;
  }

  frame->points = optimiser.optimise(frame->points);
  send(frame);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "optimise",
  "Optimise",
  "Optimise frames for laser scanners",
  "vector",
  {
    { "max-distance", { "Maximum distance between points", Value::Type::number } },
    { "max-angle",    { "Angle at a vertex before adding points",
          Value::Type::number } },
    { "vertex-repeats", { "Number of points to add at a sharp vertex",
          Value::Type::number } }
  },
  { "VectorFrame" }, // inputs
  { "VectorFrame" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(OptimiseFilter, module)
