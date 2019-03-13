//==========================================================================
// ViGraph dataflow module: laser/filters/infill-lines/infill-lines.cc
//
// Fills in lines to get constant brightness on laser scanning
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../vector/vector-module.h"
#include "vg-laser.h"
#include <cmath>

namespace {

//==========================================================================
// InfillLines filter
class InfillLinesFilter: public FrameFilter
{
  double max_distance_lit;
  double max_distance_blanked;
  Laser::Optimiser optimiser;

  // Filter/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void accept(FramePtr frame) override;

public:
  // Construct
  InfillLinesFilter(const Dataflow::Module *module,
                    const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
//  <infill-lines max-distance="0.01"/>
InfillLinesFilter::InfillLinesFilter(const Dataflow::Module *module,
                                     const XML::Element& config):
  FrameFilter(module, config)
{
  max_distance_lit = config.get_attr_real("lit");
  max_distance_blanked = config.get_attr_real("blanked");
}

//--------------------------------------------------------------------------
// Set a control property
void InfillLinesFilter::set_property(const string& property,
                                            const SetParams& sp)
{
  if (property == "lit")
    update_prop(max_distance_lit, sp);
  else if (property == "blanked")
    update_prop(max_distance_blanked, sp);
}

//--------------------------------------------------------------------------
// Process some data
void InfillLinesFilter::accept(FramePtr frame)
{
  frame->points = optimiser.infill_lines(frame->points, max_distance_lit,
                                         max_distance_blanked);
  send(frame);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "infill-lines",
  "Infill lines",
  "Adds a spread of points to lines to get constant brightness in laser scans",
  "laser",
  {
    { "lit", { "Maximum distance between lit points",
          Value::Type::number, true } },
    { "blanked", { "Maximum distance between blanked points",
          Value::Type::number, true } }
  },
  { "VectorFrame" }, // inputs
  { "VectorFrame" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(InfillLinesFilter, module)
