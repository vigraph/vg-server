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

const int default_repeats = 5;

//==========================================================================
// AddBlankingAnchors filter
class AddBlankingAnchorsFilter: public FrameFilter
{
  int repeats;
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
  repeats = config.get_attr_int("repeats", default_repeats);
}

//--------------------------------------------------------------------------
// Set a control property
void AddBlankingAnchorsFilter::set_property(const string& property,
                                            const SetParams& sp)
{
  if (property == "repeats")
    update_prop_int(repeats, sp);
}

//--------------------------------------------------------------------------
// Process some data
void AddBlankingAnchorsFilter::accept(FramePtr frame)
{
  frame->points = optimiser.add_blanking_anchors(frame->points, repeats);
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
    { "repeats", { "Number of points to add at blanking start/end",
          Value::Type::number, true } }
  },
  { "VectorFrame" }, // inputs
  { "VectorFrame" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(AddBlankingAnchorsFilter, module)
