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
// Constructor - get acceptors as well as Element stuff
// Attribute acceptor or sub-elements <acceptor id>
Generator::Generator(const Module *_module, const XML::Element& config):
  Element(_module, config)
{
  const auto& id = config["acceptor"];
  if (!id.empty()) attach(id);

  for(const auto a_e: config.get_children("acceptor"))
  {
    const auto& id = (*a_e)["id"];
    if (!id.empty()) attach(id);
  }
}

//------------------------------------------------------------------------
// Send data to all acceptors
void Generator::send(DataPtr data)
{
  bool first = true;
  for(const auto& it: acceptors)
  {
    if (first)
      it.second->accept(data);  // Send original
    else
      it.second->accept(clone(data));  // Send a copy
    first = false;
  }
}

//------------------------------------------------------------------------
// Get state as JSON
JSON::Value Generator::get_json(const string& path) const
{
  JSON::Value json = Element::get_json(path);
  if (!acceptors.empty())
  {
    JSON::Value& outsj = json.set("outputs", JSON::Value(JSON::Value::OBJECT));

    // ! currently default only, until we have multiple output types
    JSON::Value& defj = outsj.set("default", JSON::Value(JSON::Value::ARRAY));
    for(const auto& it: acceptors)
    {
      JSON::Value& oj = defj.add(JSON::Value(JSON::Value::OBJECT));
      oj.set("element", it.first);
      oj.set("prop", "default");
    }
  }
  return json;
}

}} // namespaces
