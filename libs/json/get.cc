//==========================================================================
// ViGraph JSON: get.cc
//
// JSON get functions
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-json.h"

namespace ViGraph { namespace JSON {

void GetVisitor::visit(Dataflow::Engine&)
{
}

unique_ptr<Dataflow::Visitor> GetVisitor::getSubGraphVisitor()
{
  auto git = json.o.find("graphs");
  if (git == json.o.end())
    git = json.o.emplace("graphs", Value::Type::ARRAY).first;
  auto& graphs = git->second;
  return make_unique<GetVisitor>(graphs.add(Value::Type::OBJECT));
}

void GetVisitor::visit(Dataflow::Graph&)
{
}

unique_ptr<Dataflow::Visitor> GetVisitor::getSubElementVisitor(const string&)
{
  auto eit = json.o.find("elements");
  if (eit == json.o.end())
    eit = json.o.emplace("elements", Value::Type::ARRAY).first;
  auto& elements = eit->second;
  return make_unique<GetVisitor>(elements.add(Value::Type::OBJECT));
}

void GetVisitor::visit(Dataflow::Element& element)
{
  json.put("id", element.id);
  json.put("type", element.module.type());
  if (!element.module.settings.empty())
  {
    auto& settingsj = json.put("settings", Value::Type::OBJECT);
    for (const auto& sit: element.module.settings)
    {
      const auto& name = sit.first;
      const auto& setting = sit.second;
      settingsj.put(name, setting.get_json(element));
    }
  }
  if (!element.module.inputs.empty())
  {
    auto& inputsj = json.put("inputs", Value::Type::OBJECT);
    for (const auto& iit: element.module.inputs)
    {
      const auto& name = iit.first;
      const auto& input = iit.second;
      inputsj.put(name, input.get_json(element));
    }
  }
  if (!element.module.outputs.empty())
  {
    auto& outputsj = json.put("outputs", Value::Type::OBJECT);
    for (const auto& oit: element.module.outputs)
    {
      const auto& name = oit.first;
      const auto& output = oit.second.get(element);
      const auto& conns = output.get_connections();
      if (conns.empty())
        continue;
      auto& outputj = outputsj.put(name, Value::Type::ARRAY);
      for (const auto& conn: conns)
      {
        auto& connj = outputj.add(Value::Type::OBJECT);
        connj.put("element", conn.element->id);
        connj.put("input", conn.element->module.get_input_id(*conn.element,
                                                             *conn.input));
      }
    }
  }
}

/* path visit graph
  else
  {
    // Split path and use first (or only) as ID, pass rest (or empty) down
    auto bits = Text::split(path, '/', false, 2);
    const auto it = elements.find(bits[0]);
    if (it == elements.end())
      throw runtime_error("No such sub-element "+bits[0]+" in graph");

    // Return bare value (or INVALID) up, undecorated
    return get_element(it->second, bits.size()>1 ? bits[1] : "");
  }
}
*/

}} // namespaces
