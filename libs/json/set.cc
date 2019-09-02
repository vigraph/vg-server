//==========================================================================
// ViGraph JSON: set.cc
//
// JSON set functions
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-json.h"

namespace ViGraph { namespace JSON {

void SetVisitor::visit(Dataflow::Engine&)
{
}

unique_ptr<Dataflow::Visitor> SetVisitor::getSubGraphVisitor()
{
  auto it = json.o.find("graphs");
  if (it == json.o.end())
    return {};
  const auto& graphsj = it->second;
  if (graphsj.a.empty())
    return {};
  return make_unique<SetVisitor>(graphsj.a.front(), scope_graph);
}

void SetVisitor::visit(Dataflow::Graph& graph)
{
  scope_graph = &graph;
  graph.shutdown();
  auto it = json.o.find("elements");
  if (it == json.o.end())
    return;
  const auto& elementsj = it->second;
  for (const auto& elementj: elementsj.a)
  {
    auto idit = elementj.o.find("id");
    if (idit == elementj.o.end())
      throw runtime_error("Graph element requires an 'id'");
    const auto& id = idit->second.as_str();
    if (id.empty())
      throw runtime_error("Graph element requires an 'id'");
    auto typeit = elementj.o.find("type");
    if (typeit == elementj.o.end())
      throw runtime_error("Graph element requires a 'type'");
    const auto& type = typeit->second.as_str();
    if (type.empty())
      throw runtime_error("Graph element requires a 'type'");
    graph.add_element(type, id);
    sub_element_json[id] = &elementj;
  }
}

unique_ptr<Dataflow::Visitor> SetVisitor::getSubElementVisitor(const string& id)
{
  auto it = sub_element_json.find(id);
  if (it == sub_element_json.end())
    return {};
  return make_unique<SetVisitor>(*it->second, scope_graph);
}

void SetVisitor::visit(Dataflow::Element& element)
{
  auto it = json.o.find("settings");
  if (it != json.o.end() && it->second.type == Value::Type::OBJECT)
  {
    for (const auto sit: it->second.o)
    {
      const auto& name = sit.first;
      const auto& value = sit.second;

      auto msit = element.module.settings.find(name);
      if (msit == element.module.settings.end())
      {
        Log::Error log;
        log << "Unknown setting '" << name << "' on element "
            << element.id << endl;
        continue;
      }

      msit->second.set_json(element, value);
    }
  }

  element.setup();

  it = json.o.find("inputs");
  if (it != json.o.end() && it->second.type == Value::Type::OBJECT)
  {
    for (const auto iit: it->second.o)
    {
      const auto& name = iit.first;
      const auto& value = iit.second;

      auto miit = element.module.inputs.find(name);
      if (miit == element.module.inputs.end())
      {
        Log::Error log;
        log << "Unknown input '" << name << "' on element "
            << element.id << endl;
        continue;
      }

      miit->second.set_json(element, value);
    }
  }
  if (scope_graph)
  {
    it = json.o.find("outputs");
    if (it != json.o.end() && it->second.type == Value::Type::OBJECT)
    {
      for (const auto oit: it->second.o)
      {
          const auto& name = oit.first;
        for (const auto& connection: oit.second.a)
        {
          const auto& iname = connection["element"].as_str();
          const auto& iinput = connection["input"].as_str();
          auto ielement = scope_graph->get_element(iname);
          if (!ielement)
          {
            Log::Error log;
            log << "Unknown element '" << iname << "' for element "
                << element.id << " output connection on " << name << endl;
            continue;
          }

          if (!element.connect(name, *ielement, iinput))
          {
            Log::Error log;
            log << "Could not connect " << element.id << "." << name << " to "
                << ielement->id << "." << iinput << endl;
            continue;
          }
        }
      }
    }
  }
}

}} // namespaces
