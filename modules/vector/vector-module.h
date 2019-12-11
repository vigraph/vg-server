//==========================================================================
// ViGraph vector graphics modules: vector-module.h
//
// Common definitions for vector graphics modules
//
// Copyright (c) 2017-2019 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VIGRAPH_VECTOR_MODULE_H
#define __VIGRAPH_VECTOR_MODULE_H

#include "../module.h"
#include "vg-geometry.h"
#include "vg-dataflow.h"

using namespace ViGraph::Geometry;

namespace ViGraph { namespace Module { namespace Vector {

//==========================================================================
// Animation frame
struct Frame
{
  vector<Point> points;

  Frame() {}
  Frame(const Frame& o): points(o.points) {}

  Frame& operator+=(const Frame& o)
  {
    points.insert(points.begin(), o.points.begin(), o.points.end());
    return *this;
  }
};

typedef shared_ptr<Frame> FramePtr;

}}} //namespaces

using namespace ViGraph::Module::Vector;

//==========================================================================
// JSON conversions
namespace ViGraph { namespace Dataflow {

template<> inline
string get_module_type<Frame>() { return "frame"; }

template<> inline void set_from_json(Frame& frame,
                                     const JSON::Value& json)
{
  if (json.type == JSON::Value::OBJECT)
  {
    for(const JSON::Value& jp: json["points"].a)
    {
      frame.points.push_back(Point(jp["x"].as_float(),
                                   jp["y"].as_float(),
                                   jp["z"].as_float(),
                                   Colour::RGB(jp["c"].as_str())));
    }
  }
}

template<> inline JSON::Value get_as_json(const Frame& frame)
{
  JSON::Value value{JSON::Value::OBJECT};
  JSON::Value& points = value.put("points", JSON::Value::ARRAY);
  for(const auto& p: frame.points)
  {
    JSON::Value &jp = points.add(JSON::Value::OBJECT);
    jp.set("x", p.x).set("y", p.y).set("z", p.z).set("c", p.c.str());
  }

  return value;
}


}} //namespaces

#endif // !__VIGRAPH_VECTOR_MODULE_H
