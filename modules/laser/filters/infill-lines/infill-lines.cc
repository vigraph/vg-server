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

const double default_max_distance = 0.01;

//==========================================================================
// InfillLines filter
class InfillLinesFilter: public FrameFilter
{
  double max_distance;
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
  Element(module, config), FrameFilter(module, config)
{
  max_distance = config.get_attr_real("max-distance", default_max_distance);
}

//--------------------------------------------------------------------------
// Set a control property
void InfillLinesFilter::set_property(const string& property,
                                            const SetParams& sp)
{
  if (property == "max-distance")
    update_prop(max_distance, sp);
}

//--------------------------------------------------------------------------
// Process some data
void InfillLinesFilter::accept(FramePtr frame)
{
  frame->points = optimiser.infill_lines(frame->points, max_distance);
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
    { "max-distance", { "Maximum distance between points",
          Value::Type::number, true } }
  },
  { "VectorFrame" }, // inputs
  { "VectorFrame" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(InfillLinesFilter, module)
