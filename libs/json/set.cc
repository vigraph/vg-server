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
  return make_unique<SetVisitor>(engine, json, scope_graph);
}

void SetVisitor::visit(Dataflow::Graph& graph)
{
  graph.shutdown();

  // Contained elements
  auto& elements = json.get("elements");
  if (!!elements)
  {
    for (const auto& it: elements.o)
    {
      const auto& id = it.first;
      const auto& elementj = it.second;
      if (id.empty())
        throw runtime_error("Graph element requires an 'id'");
      const auto &typej = elementj.get("type");
      if (!typej)
        throw runtime_error("Graph element requires a 'type'");
      const auto& type = typej.as_str();
      if (type.empty())
        throw runtime_error("Graph element requires a 'type'");

      auto element = engine.create(type, id);
      if (element)
      {
        graph.add(element);
        // Setup element
        auto& module = element->get_module();
        auto it = elementj.o.find("settings");
        if (it != elementj.o.end() && it->second.type == Value::Type::OBJECT)
        {
          for (const auto sit: it->second.o)
          {
            const auto& name = sit.first;
            const auto& value = sit.second;

            auto setting = module.get_setting(name);
            if (!setting)
            {
              Log::Error log;
              log << "Unknown setting '" << name << "' on element "
                  << element->id << endl;
              continue;
            }

            setting->set_json(*element, value);
          }
        }

        element->setup();

        // Add to our element cache for later visiting
        sub_element_json[id] = &elementj;
      }
    }
  }

  // Inputs
  auto& inputsj = json.get("inputs");
  if (!!inputsj && inputsj.type == Value::Type::OBJECT)
  {
    for (const auto& iit: inputsj.o)
    {
      const auto& iid = iit.first;
      const auto& inputj = iit.second;
      if (iid.empty())
        throw runtime_error("Graph input pin requires an 'id'");
      const auto& typej = inputj.get("type");
      if (!typej)
        throw runtime_error("Graph input pin requires a 'type'");
      const auto& type = typej.as_str();
      if (type.empty())
        throw runtime_error("Graph input pin requires a 'type'");

      graph.add_input_pin(iid, iid, "input");
    }
  }

  // Outputs
  auto& outputsj = json.get("outputs");
  if (!!outputsj && outputsj.type == Value::Type::OBJECT)
  {
    for (const auto& oit: outputsj.o)
    {
      const auto& oid = oit.first;
      const auto& outputj = oit.second;
      if (oid.empty())
        throw runtime_error("Graph output pin requires an 'id'");
      const auto& typej = outputj.get("type");
      if (!typej)
        throw runtime_error("Graph output pin requires a 'type'");
      const auto& type = typej.as_str();
      if (type.empty())
        throw runtime_error("Graph output pin requires a 'type'");

      graph.add_output_pin(oid, oid, "output");

      // Connections
      const auto& connectionsj = outputj.get("connections");
      if (!!connectionsj && connectionsj.type == Value::Type::ARRAY)
      {
        for (const auto& connj: connectionsj.a)
        {
          const auto& iname = connj["element"].as_str();
          const auto& iinput = connj["input"].as_str();
          auto ielement = scope_graph->get_element(iname);
          if (!ielement)
          {
            Log::Error log;
            log << "Unknown element '" << iname << "' for element "
                << graph.id << " output connection on " << oid << endl;
            continue;
          }

          if (!graph.connect(oid, *ielement, iinput))
          {
            Log::Error log;
            log << "Could not connect " << graph.id << "." << oid << " to "
                << ielement->id << "." << iinput << endl;
            continue;
          }
        }
      }
    }
  }

  scope_graph = &graph;
}

unique_ptr<Dataflow::Visitor> SetVisitor::getSubElementVisitor(const string& id)
{
  auto it = sub_element_json.find(id);
  if (it == sub_element_json.end())
    return {};
  return make_unique<SetVisitor>(engine, *it->second, scope_graph);
}

void SetVisitor::visit(Dataflow::Element& element)
{
  auto& module = element.get_module();
  auto& inputsj = json.get("inputs");
  if (!!inputsj && inputsj.type == Value::Type::OBJECT)
  {
    for (const auto iit: inputsj.o)
    {
      const auto& name = iit.first;
      const auto& inputj = iit.second;

      auto input = module.get_input(name);
      if (!input)
      {
        Log::Error log;
        log << "Unknown input '" << name << "' on element "
            << element.id << endl;
        continue;
      }

      const auto& valuej = inputj.get("value");
      if (!!valuej)
        input->set_json(element, valuej);
    }
  }
  if (scope_graph)
  {
    auto outputsj = json.get("outputs");
    if (!!outputsj && outputsj.type == Value::Type::OBJECT)
    {
      for (const auto oit: outputsj.o)
      {
        const auto& id = oit.first;
        const auto& outputj = oit.second;

        const auto& connectionsj = outputj.get("connections");
        if (!!connectionsj && connectionsj.type == Value::Type::ARRAY)
        {
          for (const auto& connectionj: connectionsj.a)
          {
            const auto& iid = connectionj["element"].as_str();
            const auto& iinput = connectionj["input"].as_str();
            auto ielement = (iid == scope_graph->id
                             ? scope_graph : scope_graph->get_element(iid));
            if (!ielement)
            {
              Log::Error log;
              log << "Unknown element '" << iid << "' for element "
                  << element.id << " output connection on " << id << endl;
              continue;
            }

            if (!element.connect(id, *ielement, iinput))
            {
              Log::Error log;
              log << "Could not connect " << element.id << "." << id << " to "
                  << ielement->id << "." << iinput << endl;
              continue;
            }
          }
        }
      }
    }
  }
}

}} // namespaces
