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
  int blanking_repeats;
  bool reorder;
  Laser::Optimiser optimiser;
  Laser::Reorderer reorderer;

  // Filter/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void accept(FramePtr frame) override;

public:
  // Construct
  OptimiseFilter(const Dataflow::Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
//  <optimise max-distance="0.001" max-angle="30" vertex-repeats="3"
//            blanking-repeats="5" reorder="true"/>
OptimiseFilter::OptimiseFilter(const Dataflow::Module *module,
                               const XML::Element& config):
  Element(module, config), FrameFilter(module, config)
{
  max_distance = config.get_attr_real("max-distance");
  optimiser.enable_infill(max_distance);

  max_angle = config.get_attr_real("max-angle")*pi/180;
  vertex_repeats = config.get_attr_int("vertex-repeats");
  optimiser.enable_vertex_repeats(max_angle, vertex_repeats);

  blanking_repeats = config.get_attr_int("blanking-repeats");
  optimiser.enable_blanking_repeats(blanking_repeats);

  reorder = config.get_attr_bool("reorder");
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
  else if (property == "blanking-repeats")
  {
    update_prop_int(blanking_repeats, sp);
    optimiser.enable_blanking_repeats(blanking_repeats);
  }
}

//--------------------------------------------------------------------------
// Process some data
void OptimiseFilter::accept(FramePtr frame)
{
  // Fast null operation
  if (!max_distance && !max_angle && !blanking_repeats && !reorder)
  {
    send(frame);
    return;
  }

  frame->points = optimiser.optimise(frame->points);

  if (reorder)
    frame->points = reorderer.reorder(frame->points);

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
    { "max-distance", { "Maximum distance between points",
          Value::Type::number, true } },
    { "max-angle",    { "Angle at a vertex before adding points",
          Value::Type::number, true } },
    { "vertex-repeats", { "Number of points to add at a sharp vertex",
          Value::Type::number, true } },
    { "blanking-repeats", { "Number of points to add at blanking start/end",
          Value::Type::number, true } },
    { "reorder", { "Whether to reorder segments to minimise scan distance",
          Value::Type::boolean } }
  },
  { "VectorFrame" }, // inputs
  { "VectorFrame" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(OptimiseFilter, module)
