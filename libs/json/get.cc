//==========================================================================
// ViGraph JSON: get.cc
//
// JSON get functions
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-json.h"

namespace ViGraph { namespace JSON {

bool GetVisitor::visit(const Dataflow::Engine&,
                       const Dataflow::Path&, unsigned)
{
  return true;
}

unique_ptr<Dataflow::ReadVisitor> GetVisitor::get_root_graph_visitor()
{
  return make_unique<GetVisitor>(json);
}

bool GetVisitor::visit(const Dataflow::Graph& graph,
                       const Dataflow::Path&, unsigned)
{
  auto& module = graph.get_module();
  json.put("type", module.get_full_type());
  if (module.has_outputs())
  {
    module.for_each_output([this]
                           (const string& id, const Dataflow::OutputMember&)
    {
      this->output_pins.insert(id);
    });
  }
  return true;
}

bool GetVisitor::visit(const Dataflow::Clone& clone,
                       const Dataflow::Path&, unsigned)
{
  json.set("type", "core:clone");
  json.set("number", clone.number.get());
  return true;
}

unique_ptr<Dataflow::ReadVisitor>
    GetVisitor::get_sub_element_visitor(const string& id,
                                        bool visit,
                                        const Dataflow::Graph&)
{
  if (visit)
  {
    auto eit = json.o.find("elements");
    if (eit == json.o.end())
      eit = json.o.emplace("elements", Value::Type::OBJECT).first;
    auto& elements = eit->second;
    auto no_connections = output_pins.find(id) != output_pins.end();
    return make_unique<GetVisitor>(elements.put(id, Value::Type::OBJECT),
                                   no_connections);
  }
  else
  {
    return make_unique<GetVisitor>(json);
  }
}

bool GetVisitor::visit(const Dataflow::Element& element,
                       const Dataflow::Path&, unsigned)
{
  auto& module = element.get_module();
  json.put("type", module.get_full_type());
  return true;
}

unique_ptr<Dataflow::ReadVisitor>
    GetVisitor::get_element_setting_visitor(const string& id, bool visit)
{
  if (visit)
  {
    auto eit = json.o.find("settings");
    if (eit == json.o.end())
      eit = json.o.emplace("settings", Value::Type::OBJECT).first;
    auto& settings = eit->second;
    return make_unique<GetVisitor>(settings.put(id, Value::Type::OBJECT));
  }
  else
  {
    return make_unique<GetVisitor>(json);
  }
}

bool GetVisitor::visit(const Dataflow::GraphElement& element,
                       const Dataflow::SettingMember& setting,
                       const Dataflow::Path&, unsigned)
{
  json.set("type", setting.get_type());
  json.set("value", setting.get_json(element));
  return true;
}

unique_ptr<Dataflow::ReadVisitor>
    GetVisitor::get_element_input_visitor(const string& id, bool visit)
{
  if (visit)
  {
    auto eit = json.o.find("inputs");
    if (eit == json.o.end())
      eit = json.o.emplace("inputs", Value::Type::OBJECT).first;
    auto& inputs = eit->second;
    return make_unique<GetVisitor>(inputs.put(id, Value::Type::OBJECT));
  }
  else
  {
    return make_unique<GetVisitor>(json);
  }
}

bool GetVisitor::visit(const Dataflow::GraphElement& element,
                       const Dataflow::InputMember& input,
                       const Dataflow::Path&, unsigned)
{
  json.set("type", input.get_type());
  json.set("value", input.get_json(element));
  return true;
}

unique_ptr<Dataflow::ReadVisitor>
    GetVisitor::get_element_output_visitor(const string& id, bool visit)
{
  if (visit)
  {
    if (no_connections)
      return nullptr;
    auto eit = json.o.find("outputs");
    if (eit == json.o.end())
      eit = json.o.emplace("outputs", Value::Type::OBJECT).first;
    auto& outputs = eit->second;
    return make_unique<GetVisitor>(outputs.put(id, Value::Type::OBJECT));
  }
  else
  {
    return make_unique<GetVisitor>(json);
  }
}

bool GetVisitor::visit(const Dataflow::GraphElement& element,
                       const Dataflow::OutputMember& output,
                       const Dataflow::Path&, unsigned)
{
  json.put("type", output.get_type());
  auto& op = output.get(element);
  const auto& conns = op.get_connections();
  if (conns.empty())
    return true;
  auto& connectionsj = json.put("connections", Value::Type::ARRAY);
  for (const auto& conn: conns)
  {
    auto& connj = connectionsj.add(Value::Type::OBJECT);
    connj.put("element", conn.element->get_id());
    auto& imodule = conn.element->get_module();
    connj.put("input", imodule.get_input_id(*conn.element, *conn.input));
  }
  return true;
}

bool GetVisitor::visit_graph_input_or_output(const Dataflow::Graph&,
                                             const string&,
                                             bool)
{
  return true;
}

}} // namespaces
