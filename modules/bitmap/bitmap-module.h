//==========================================================================
// ViGraph bitmap graphics modules: bitmap-module.h
//
// Common definitions for bitmap graphics modules
//
// Copyright (c) 2017-2019 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VIGRAPH_BITMAP_MODULE_H
#define __VIGRAPH_BITMAP_MODULE_H

#include "../module.h"
#include "vg-dataflow.h"
#include "vg-bitmap.h"

using namespace ViGraph::Geometry;

//==========================================================================
// JSON conversions
namespace ViGraph { namespace Dataflow {

template<> inline
string get_module_type<Bitmap::Group>() { return "frame"; }

template<> inline void set_from_json(Bitmap::Group& bitmap,
                                     const JSON::Value& json)
{
  if (json.type == JSON::Value::ARRAY)
  {
    for(const JSON::Value& bm: json.a)
    {
      Bitmap::Rectangle rect;
      // !!! Read bitmap data
      bitmap.items.push_back(Bitmap::Group::Item(
                               Vector(bm["x"].as_float(),
                                      bm["y"].as_float(),
                                      bm["z"].as_float()),
                               rect));
    }
  }
}

template<> inline JSON::Value get_as_json(const Bitmap::Group& bitmap)
{
  JSON::Value value{JSON::Value::ARRAY};
  for(const auto& item: bitmap.items)
  {
    auto& bm = value.add(JSON::Value::OBJECT);
    bm.set("x", item.pos.x);
    bm.set("y", item.pos.y);
    bm.set("z", item.pos.z);

    // !!! Bitmap data itself
  }
  return value;
}

}} //namespaces

#endif // !__VIGRAPH_BITMAP_MODULE_H
