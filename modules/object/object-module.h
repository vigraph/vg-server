//==========================================================================
// ViGraph object modules: object-module.h
//
// Common definitions for object modules
//
// Copyright (c) 2021 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VIGRAPH_OBJECT_MODULE_H
#define __VIGRAPH_OBJECT_MODULE_H

#include "../module.h"
#include "vg-dataflow.h"
#include "ot-json.h"

namespace ViGraph { namespace Module { namespace Object {

//==========================================================================
// Object data
struct Data
{
  JSON::Value json{JSON::Value::OBJECT};

  // Combine with another one
  Data& operator+=(const Data& other)
  {
    // Overlay all other's properties onto ours
    for(const auto& p: other.json.o)
      json.set(p.first, p.second);

    return *this;
  }
};

typedef shared_ptr<Data> DataPtr;

}}} //namespaces

using namespace ViGraph::Module::Object;

//==========================================================================
// JSON conversions
namespace ViGraph { namespace Dataflow {

template<> inline
string get_module_type<Data>() { return "object"; }

template<> inline void set_from_json(Data& data,
                                     const JSON::Value& json)
{
  if (json.type == JSON::Value::OBJECT)
    data.json = json;
}

template<> inline JSON::Value get_as_json(const Data& data)
{
  return data.json;
}

}} //namespaces

#endif // !__VIGRAPH_OBJECT_MODULE_H
