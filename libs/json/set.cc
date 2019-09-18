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
  return make_unique<SetVisitor>(engine, graphsj.a.front(), scope_graph);
}

void SetVisitor::visit(Dataflow::Graph& graph)
{
  graph.shutdown();

  // Output pins
  auto it = json.o.find("output-pins");
  if (it != json.o.end())
  {
    const auto& outputpinsj = it->second;
    for (const auto& outputpinj: outputpinsj.a)
    {
      auto idit = outputpinj.o.find("id");
      if (idit == outputpinj.o.end())
        throw runtime_error("Graph output pin requires an 'id'");
      const auto& id = idit->second.as_str();
      if (id.empty())
        throw runtime_error("Graph output pin requires an 'id'");
      auto typeit = outputpinj.o.find("type");
      if (typeit == outputpinj.o.end())
        throw runtime_error("Graph output pin requires a 'type'");
      const auto& type = typeit->second.as_str();
      if (type.empty())
        throw runtime_error("Graph output pin requires a 'type'");

      auto pin = shared_ptr<Dataflow::GraphElement>(engine.create(type, id));
      if (!pin)
        throw runtime_error("Could not create pin of type '" + type + "'");

      graph.add_output_pin(id, pin);

      // Connections
      auto oit = outputpinj.o.find("outputs");
      if (oit != outputpinj.o.end())
      {
        const auto& outputsj = oit->second;
        for (const auto& outputj: outputsj.a)
        {
          const auto& iname = outputj["element"].as_str();
          const auto& iinput = outputj["input"].as_str();
          auto ielement = scope_graph->get_element(iname);
          if (!ielement)
          {
            Log::Error log;
            log << "Unknown element '" << iname << "' for element "
                << graph.id << " output connection on " << id << endl;
            continue;
          }

          if (!graph.connect(id, *ielement, iinput))
          {
            Log::Error log;
            log << "Could not connect " << graph.id << "." << id << " to "
                << ielement->id << "." << iinput << endl;
            continue;
          }
        }
      }
    }
  }

  // Contained elements
  it = json.o.find("elements");
  if (it != json.o.end())
  {
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

  // Input pins
  it = json.o.find("input-pins");
  if (it != json.o.end())
  {
    const auto& inputpinsj = it->second;
    for (const auto& inputpinj: inputpinsj.a)
    {
      auto idit = inputpinj.o.find("id");
      if (idit == inputpinj.o.end())
        throw runtime_error("Graph input pin requires an 'id'");
      const auto& id = idit->second.as_str();
      if (id.empty())
        throw runtime_error("Graph input pin requires an 'id'");
      auto typeit = inputpinj.o.find("type");
      if (typeit == inputpinj.o.end())
        throw runtime_error("Graph input pin requires a 'type'");
      const auto& type = typeit->second.as_str();
      if (type.empty())
        throw runtime_error("Graph input pin requires a 'type'");

      auto pin = shared_ptr<Dataflow::GraphElement>(engine.create(type, id));
      if (!pin)
        throw runtime_error("Could not create pin of type '" + type + "'");

      graph.add_input_pin(id, pin);

      // Connections
      auto oit = inputpinj.o.find("outputs");
      if (oit != inputpinj.o.end())
      {
        const auto& outputsj = oit->second;
        for (const auto& outputj: outputsj.a)
        {
          const auto& iname = outputj["element"].as_str();
          const auto& iinput = outputj["input"].as_str();
          auto ielement = graph.get_element(iname);
          if (!ielement)
          {
            Log::Error log;
            log << "Unknown element '" << iname << "' for element "
                << graph.id << " output connection on " << id << endl;
            continue;
          }

          if (!graph.connect(id, *ielement, iinput))
          {
            Log::Error log;
            log << "Could not connect " << graph.id << "." << id << " to "
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
  auto it = json.o.find("inputs");
  if (it != json.o.end() && it->second.type == Value::Type::OBJECT)
  {
    for (const auto iit: it->second.o)
    {
      const auto& name = iit.first;
      const auto& value = iit.second;

      auto input = module.get_input(name);
      if (!input)
      {
        Log::Error log;
        log << "Unknown input '" << name << "' on element "
            << element.id << endl;
        continue;
      }

      input->set_json(element, value);
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
          auto ielement = (iname == scope_graph->id
                           ? scope_graph : scope_graph->get_element(iname));
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
