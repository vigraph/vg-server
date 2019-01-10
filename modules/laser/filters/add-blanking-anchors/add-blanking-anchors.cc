//==========================================================================
// ViGraph dataflow module:
//  laser/filters/add-blanking-anchors/add-blanking-anchors.cc
//
// Adds anchor points before/after blanking moves
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../vector/vector-module.h"
#include "vg-laser.h"
#include <cmath>

namespace {

const int default_leading = 5;
const int default_trailing = 5;

//==========================================================================
// AddBlankingAnchors filter
class AddBlankingAnchorsFilter: public FrameFilter
{
  int leading;
  int trailing;
  Laser::Optimiser optimiser;

  // Filter/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void accept(FramePtr frame) override;

public:
  // Construct
  AddBlankingAnchorsFilter(const Dataflow::Module *module,
                           const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
//  <AddBlankingAnchors repeats="5"/>
AddBlankingAnchorsFilter::AddBlankingAnchorsFilter(
                                         const Dataflow::Module *module,
                                         const XML::Element& config):
  Element(module, config), FrameFilter(module, config)
{
  leading = config.get_attr_int("leading", default_leading);
  trailing = config.get_attr_int("trailing", default_trailing);
}

//--------------------------------------------------------------------------
// Set a control property
void AddBlankingAnchorsFilter::set_property(const string& property,
                                            const SetParams& sp)
{
  if (property == "leading")
    update_prop_int(leading, sp);
  else if (property == "trailing")
    update_prop_int(trailing, sp);
}

//--------------------------------------------------------------------------
// Process some data
void AddBlankingAnchorsFilter::accept(FramePtr frame)
{
  frame->points = optimiser.add_blanking_anchors(frame->points, leading,
                                                 trailing);
  send(frame);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "add-blanking-anchors",
  "Add blanking anchors",
  "Add anchors points around blanking for laser scanners",
  "laser",
  {
    { "leading", { "Number of points to add at start of lit segment",
          Value::Type::number, true } },
    { "trailing", { "Number of points to add at end of lit segment",
          Value::Type::number, true } }
  },
  { "VectorFrame" }, // inputs
  { "VectorFrame" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(AddBlankingAnchorsFilter, module)
