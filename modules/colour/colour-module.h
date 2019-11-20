//==========================================================================
// ViGraph colour graphics modules: colour-module.h
//
// Common definitions for colour modules
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VIGRAPH_COLOUR_MODULE_H
#define __VIGRAPH_COLOUR_MODULE_H

#include "../module.h"
#include "vg-dataflow.h"
#include "vg-colour.h"

//==========================================================================
// JSON conversions
namespace ViGraph { namespace Dataflow {

template<> inline
string get_module_type<Colour::RGB>() { return "colour"; }

template<> inline void set_from_json(Colour::RGB& c,
                                     const JSON::Value& json)
{
  if (json.type == JSON::Value::OBJECT)
  {
    c = Colour::RGB(json["r"].as_float(),
                    json["g"].as_float(),
                    json["b"].as_float());
  }
}

template<> inline JSON::Value get_as_json(const Colour::RGB& c)
{
  JSON::Value value{JSON::Value::OBJECT};
  value.set("r", c.r);
  value.set("g", c.g);
  value.set("b", c.b);
  return value;
}

}} //namespaces

#endif // !__VIGRAPH_COLOUR_MODULE_H
