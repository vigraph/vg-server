//==========================================================================
// ViGraph dataflow machines: generator.cc
//
// Generator implementation
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-dataflow.h"

namespace ViGraph { namespace Dataflow {

//------------------------------------------------------------------------
// Get state as JSON
JSON::Value Generator::get_json() const
{
  JSON::Value json = Element::get_json();
  if (!acceptor_id.empty())
  {
    JSON::Value& outsj = json.set("outputs", JSON::Value(JSON::Value::OBJECT));
    JSON::Value& defj = outsj.set("default", JSON::Value(JSON::Value::OBJECT));
    defj.set("element", acceptor_id);
    defj.set("prop", "default");
  }
  return json;
}

}} // namespaces
