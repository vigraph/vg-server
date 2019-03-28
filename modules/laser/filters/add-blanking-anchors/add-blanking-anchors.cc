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
public:
  int leading = default_leading;
  int trailing = default_trailing;

private:
  Laser::Optimiser optimiser;

  // Filter/Element virtuals
  void accept(FramePtr frame) override;

public:
  using FrameFilter::FrameFilter;
};

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
                   Value::Type::number,
                   &AddBlankingAnchorsFilter::leading, true } },
    { "trailing", { "Number of points to add at end of lit segment",
                    Value::Type::number,
                    &AddBlankingAnchorsFilter::trailing, true } }
  },
  { "VectorFrame" }, // inputs
  { "VectorFrame" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(AddBlankingAnchorsFilter, module)
