//==========================================================================
// ViGraph JSON: set.cc
//
// JSON set functions
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-json.h"

namespace ViGraph { namespace JSON {

bool SetVisitor::visit(Dataflow::Engine&,
                       const Dataflow::Path&, unsigned)
{
  return true;
}

unique_ptr<Dataflow::WriteVisitor> SetVisitor::get_root_graph_visitor()
{
  return make_unique<SetVisitor>(engine, json, "root", &engine.get_graph());
}

bool SetVisitor::visit(Dataflow::Graph& graph,
                       const Dataflow::Path&, unsigned)
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

        // If it's a clone info, notify clone
        if (clone)
        {
          auto info = dynamic_cast<Dataflow::CloneInfo *>(element);
          if (info)
            clone->register_info(graph, info);
        }

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
                  << element->get_id() << endl;
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
    }
  }
  return true;
}

bool SetVisitor::visit(Dataflow::Clone& clone,
                       const Dataflow::Path&, unsigned)
{
  clone.shutdown();
  auto number = json["number"].as_int();
  clone.number.set(number);
  clone.setup();
  return true;
}

unique_ptr<Dataflow::WriteVisitor>
    SetVisitor::get_sub_element_visitor(const string& id,
                                        Dataflow::Graph &scope)
{
  auto it = sub_element_json.find(id);
  if (it == sub_element_json.end())
    return {};
  return make_unique<SetVisitor>(engine, *it->second, id, &scope);
}

unique_ptr<Dataflow::WriteVisitor>
    SetVisitor::get_sub_clone_visitor(Dataflow::Clone &clone)
{
  return make_unique<SetVisitor>(engine, json, id, scope_graph, &clone);
}

bool SetVisitor::visit(Dataflow::Element&,
                       const Dataflow::Path&, unsigned)
{
  return true;
}

unique_ptr<Dataflow::WriteVisitor>
    SetVisitor::get_element_setting_visitor(const string &id)
{
  const auto& settingsj = json.get("settings");
  if (!settingsj || settingsj.type != Value::Type::OBJECT)
    return nullptr;

  const auto& settingj = settingsj.get(id);
  if (!settingj)
    return nullptr;

  return make_unique<SetVisitor>(engine, settingj, id, scope_graph);
}

bool SetVisitor::visit(Dataflow::GraphElement& element,
                       const Dataflow::SettingMember& setting,
                       const Dataflow::Path&, unsigned)
{
  const auto& valuej = json.get("value");
  if (!valuej)
    return true;
  setting.set_json(element, valuej);
  return true;
}

unique_ptr<Dataflow::WriteVisitor>
    SetVisitor::get_element_input_visitor(const string &id)
{
  const auto& inputsj = json.get("inputs");
  if (!inputsj || inputsj.type != Value::Type::OBJECT)
    return nullptr;

  const auto& inputj = inputsj.get(id);
  if (!inputj)
    return nullptr;

  return make_unique<SetVisitor>(engine, inputj, id, scope_graph);
}

bool SetVisitor::visit(Dataflow::GraphElement& element,
                       const Dataflow::InputMember& input,
                       const Dataflow::Path&, unsigned)
{
  const auto& valuej = json.get("value");
  if (!valuej)
    return true;;
  input.set_json(element, valuej);
  return true;
}

unique_ptr<Dataflow::WriteVisitor>
    SetVisitor::get_element_output_visitor(const string &id)
{
  const auto& outputsj = json.get("outputs");
  if (!outputsj || outputsj.type != Value::Type::OBJECT)
    return nullptr;

  const auto& outputj = outputsj.get(id);
  if (!outputj)
    return nullptr;

  return make_unique<SetVisitor>(engine, outputj, id, scope_graph);
}

bool SetVisitor::visit(Dataflow::GraphElement& element,
                       const Dataflow::OutputMember&,
                       const Dataflow::Path&, unsigned)
{
  if (!scope_graph)
    return true; // Can't make connections without a scope

  const auto& connectionsj = json.get("connections");
  if (!connectionsj || connectionsj.type != Value::Type::ARRAY)
    return true;

  for (const auto& connectionj: connectionsj.a)
  {
    const auto& iid = connectionj["element"].as_str();
    const auto& iinput = connectionj["input"].as_str();
    auto ielement = (iid == scope_graph->get_id()
                     ? scope_graph : scope_graph->get_element(iid));
    if (!ielement)
    {
      Log::Error log;
      log << "Unknown element '" << iid << "' for element '"
          << element.get_id() << "' output connection on '" << id
          << "' in scope of '" << scope_graph->get_id() << "'"
          << endl;
      continue;
    }

    if (!element.connect(id, *ielement, iinput))
    {
      Log::Error log;
      log << "Could not connect " << element.get_id() << "." << id
          << " to " << ielement->get_id() << "." << iinput << endl;
      continue;
    }

#if OBTOOLS_LOG_DEBUG
    Log::Debug dlog;
    dlog << "CONNECT " << element.get_id() << ":" << id << " to "
         << iid << ":" << iinput << " (scope: "
         << scope_graph->get_id() << ")" << endl;
#endif
  }
  return true;
}

}} // namespaces
