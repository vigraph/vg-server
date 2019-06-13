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
  if (acceptors.empty())
  {
    // Try to send data up to level above
    graph->send_up(data);
  }
  else
  {
    // Send copies to all except the last one - preserves non-copy where
    // there is only one, while keeping them isolated if there are more.
    auto it = acceptors.begin();
    while (it != acceptors.end())
    {
      auto a = (it++)->second;
      if (it == acceptors.end()) // Last one
        a->accept(data);         // Send original
      else
        a->accept(DataPtr(data->clone()));  // Send a copy
    }
  }
}

//------------------------------------------------------------------------
// Get state as JSON
JSON::Value Generator::get_json(const string& path) const
{
  JSON::Value json = Element::get_json(path);
  if (!acceptors.empty())
  {
    JSON::Value& outsj = json.put("outputs", JSON::Value(JSON::Value::OBJECT));

    // ! currently default only, until we have multiple output types
    JSON::Value& defj = outsj.put("default", JSON::Value(JSON::Value::ARRAY));
    for(const auto& it: acceptors)
    {
      if (!it.first.empty())
      {
        JSON::Value& oj = defj.add(JSON::Value(JSON::Value::OBJECT));
        oj.set("element", it.first);
        oj.set("prop", "default");
      }
    }
  }
  return json;
}

//------------------------------------------------------------------------
// Set acceptor from JSON
// ! output_id ignored until we have multiple output types
void Generator::set_output_from_json(const string& /*output_id*/,
                                     const JSON::Value& json)
{
  // Clear all current outputs
  acceptors.clear();
  downstreams.clear();  // !!! What about control downstreams?
                        // !!! Need to dynamically regenerate on topo calc

  if (json.type != JSON::Value::ARRAY)
    throw runtime_error("JSON to set outputs must be an array");

  for(const auto& value: json.a)
  {
    const auto& e_v = value["element"];
    if (!e_v)
      throw runtime_error("No 'element' in JSON value setting output in "+id);
    const auto& element_id = e_v.as_str();
    if (element_id.empty())
      throw runtime_error("Bad 'element' in JSON value setting output in "+id);

    Element *element = graph->get_element(element_id);
    if (!element)
      throw runtime_error("No element "+element_id+" in local graph of "+id);

    // ! Note use 'prop' when multiple output types
    // !!! Type check!
    // !!! Loop check?

    Acceptor *acceptor = dynamic_cast<Acceptor *>(element);
    if (!acceptor) throw runtime_error("Element "+element_id+" has no inputs");
    attach(element_id, acceptor);  // Note downcall to <graph> etc.
    downstreams.push_back(element);
  }
}

//------------------------------------------------------------------------
// Disconnect an element from all acceptors
void Generator::disconnect(Element *el)
{
  Element::disconnect(el);
  acceptors.erase(el->id);
}

}} // namespaces
