//==========================================================================
// ViGraph vector graphics modules: vector-module.h
//
// Common definitions for vector graphics modules
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VIGRAPH_VECTOR_MODULE_H
#define __VIGRAPH_VECTOR_MODULE_H

#include "../module.h"
#include "vg-geometry.h"
using namespace ViGraph::Geometry;

namespace ViGraph { namespace Module { namespace Vector {

//==========================================================================
// Animation frame
struct Frame: public Data
{
  vector<Point> points;
  timestamp_t timestamp;

  Frame(timestamp_t t): timestamp(t) {}
};

using FramePtr = shared_ptr<Frame>;

//==========================================================================
// Specialisations of Dataflow classes for Frame data
class FrameFilter: public Filter
{
 public:
  // Construct with XML
  FrameFilter(const Dataflow::Module *module, const XML::Element& config):
    Filter(module, config) {}

  // Accept a frame
  virtual void accept(FramePtr frame) = 0;

  // Accept data
  void accept(DataPtr data) override
  {
    // Call down with type-checked data
    accept(data.check<Frame>());
  }
};

class FrameSink: public Sink
{
 public:
  // Construct with XML
  FrameSink(const Dataflow::Module *module, const XML::Element& config):
    Sink(module, config) {}

  // Accept a frame
  virtual void accept(FramePtr frame) = 0;

  // Accept data
  void accept(DataPtr data) override
  {
    // Call down with type-checked data
    accept(data.check<Frame>());
  }
};

//==========================================================================
}}} //namespaces

using namespace ViGraph::Module::Vector;

#endif // !__VIGRAPH_VECTOR_MODULE_H
