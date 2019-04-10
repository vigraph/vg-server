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
  Frame(const Frame& o): points(o.points), timestamp(o.timestamp) {}
};

using FramePtr = shared_ptr<Frame>;

//==========================================================================
// Specialisations of Dataflow classes for Frame data
class FrameSource: public Source
{
 public:
  using Source::Source;

  // Clone frame data
  DataPtr clone(DataPtr data) override
  {
    return DataPtr(new Frame(*data.check<Frame>()));
  }
};

class FrameFilter: public Filter
{
 public:
  using Filter::Filter;

  // Accept a frame
  virtual void accept(FramePtr frame) = 0;

  // Accept data
  void accept(DataPtr data) override
  {
    // Call down with type-checked data
    accept(data.check<Frame>());
  }

  // Clone frame data
  DataPtr clone(DataPtr data) override
  {
    return DataPtr(new Frame(*data.check<Frame>()));
  }
};

class FrameSink: public Sink
{
 public:
  using Sink::Sink;

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
