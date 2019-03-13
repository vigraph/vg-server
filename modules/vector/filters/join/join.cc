//==========================================================================
// ViGraph dataflow module: filters/join/join.cc
//
// Simple filter which just combines its inputs into a single frame
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module.h"

namespace {

//==========================================================================
// Join filter
class JoinFilter: public FrameFilter
{
  shared_ptr<Frame> join{nullptr};

  // Process some data
  void accept(FramePtr frame) override;

  // Notify before and after a tick
  void pre_tick(const TickData&) override;
  void post_tick(const TickData&) override;

public:
  // Construct
  JoinFilter(const Dataflow::Module *module, const XML::Element& config):
    FrameFilter(module, config)
  {}
};

//--------------------------------------------------------------------------
// Process some data
void JoinFilter::accept(FramePtr frame)
{
  // If this is the first, just keep it
  if (!join)
  {
    // Take it over
    join = frame;
  }
  else
  {
    // Add to existing
    join->points.insert(join->points.end(), frame->points.begin(),
                        frame->points.end());
  }
}

//--------------------------------------------------------------------------
// Pre-tick setup
void JoinFilter::pre_tick(const TickData&)
{
  join.reset();
}

//--------------------------------------------------------------------------
// Post-tick flush
void JoinFilter::post_tick(const TickData& td)
{
  if (!!join)
  {
    // Reset disparate local timebases to our master
    join->timestamp = td.t;
    send(join);
  }
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "join",
  "Join",
  "Join multiple frame inputs into one",
  "vector",
  {}, // no properties
  { { "VectorFrame", true } }, // multiple inputs
  { "VectorFrame" }            // one output
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(JoinFilter, module)
