//==========================================================================
// ViGraph JSON: get.cc
//
// JSON get functions
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-json.h"

namespace ViGraph { namespace JSON {

void GetVisitor::visit(const Dataflow::Engine&,
                       const Dataflow::Path&, unsigned)
{
}

unique_ptr<Dataflow::ReadVisitor> GetVisitor::get_root_graph_visitor(
                       const Dataflow::Path&, unsigned)
{
  return make_unique<GetVisitor>(json);
}

void GetVisitor::visit(const Dataflow::Graph& graph,
                       const Dataflow::Path& path, unsigned path_index)
{
  if (!path.reached(path_index))
    return;
  auto& module = graph.get_module();
  json.put("type", module.get_full_type());
  json.set("dynamic", module.is_dynamic() ? Value::Type::TRUE_
                                          : Value::Type::FALSE_);
  if (module.has_outputs())
  {
    module.for_each_output([this]
                           (const string& id, const Dataflow::OutputMember&)
    {
      this->output_pins.insert(id);
    });
  }
}

void GetVisitor::visit(const Dataflow::Clone&,
                       const Dataflow::Path&, unsigned)
{
  const auto& module = Dataflow::clone_module;
  json.set("type", module.get_full_type());
  json.set("dynamic", module.is_dynamic() ? Value::Type::TRUE_
                                          : Value::Type::FALSE_);
}

unique_ptr<Dataflow::ReadVisitor>
    GetVisitor::get_sub_element_visitor(const Dataflow::Graph&,
                                        const string& id,
                                        const Dataflow::Path& path,
                                        unsigned path_index)
{
  if (!path.reached(path_index))
    return make_unique<GetVisitor>(json);

  auto eit = json.o.find("elements");
  if (eit == json.o.end())
    eit = json.o.emplace("elements", Value::Type::OBJECT).first;
  auto& elements = eit->second;
  auto no_connections = output_pins.find(id) != output_pins.end();
  return make_unique<GetVisitor>(elements.put(id, Value::Type::OBJECT),
                                 no_connections);
}

void GetVisitor::visit(const Dataflow::Element& element,
                       const Dataflow::Path&, unsigned)
{
  auto& module = element.get_module();
  json.put("type", module.get_full_type());
  json.set("dynamic", module.is_dynamic() ? Value::Type::TRUE_
                                          : Value::Type::FALSE_);
}

unique_ptr<Dataflow::ReadVisitor>
    GetVisitor::get_element_setting_visitor(const Dataflow::GraphElement&,
                                            const string& id,
                                            const Dataflow::Path& path,
                                            unsigned path_index)
{
  if (!path.reached(path_index))
    return make_unique<GetVisitor>(json);

  auto eit = json.o.find("settings");
  if (eit == json.o.end())
    eit = json.o.emplace("settings", Value::Type::OBJECT).first;
  auto& settings = eit->second;
  return make_unique<GetVisitor>(settings.put(id, Value::Type::OBJECT));
}

void GetVisitor::visit(const Dataflow::GraphElement& element,
                       const Dataflow::SettingMember& setting,
                       const Dataflow::Path&, unsigned)
{
  json.set("type", setting.get_type());
  json.set("value", setting.get_json(element));
}

unique_ptr<Dataflow::ReadVisitor>
    GetVisitor::get_element_input_visitor(const Dataflow::GraphElement&,
                                          const string& id,
                                          const Dataflow::Path& path,
                                          unsigned path_index)
{
  if (!path.reached(path_index))
    return make_unique<GetVisitor>(json);

  auto eit = json.o.find("inputs");
  if (eit == json.o.end())
    eit = json.o.emplace("inputs", Value::Type::OBJECT).first;
  auto& inputs = eit->second;
  return make_unique<GetVisitor>(inputs.put(id, Value::Type::OBJECT));
}

void GetVisitor::visit(const Dataflow::GraphElement& element,
                       const Dataflow::InputMember& input,
                       const Dataflow::Path&, unsigned)
{
  json.set("type", input.get_type());
  json.set("value", input.get_json(element));
  json.put("sample_rate", input.get_sample_rate(element));
}

unique_ptr<Dataflow::ReadVisitor>
    GetVisitor::get_element_output_visitor(const Dataflow::GraphElement&,
                                           const string& id,
                                           const Dataflow::Path& path,
                                           unsigned path_index)
{
  if (!path.reached(path_index))
    return make_unique<GetVisitor>(json);

  if (no_connections)
    return nullptr;
  auto eit = json.o.find("outputs");
  if (eit == json.o.end())
    eit = json.o.emplace("outputs", Value::Type::OBJECT).first;
  auto& outputs = eit->second;
  return make_unique<GetVisitor>(outputs.put(id, Value::Type::OBJECT));
}

void GetVisitor::visit(const Dataflow::GraphElement& element,
                       const Dataflow::OutputMember& output,
                       const Dataflow::Path&, unsigned)
{
  json.put("type", output.get_type());
  json.put("sample_rate", output.get_sample_rate(element));
  auto& op = output.get(element);
  const auto& conns = op.get_connections();
  if (conns.empty())
    return;
  auto& connectionsj = json.put("connections", Value::Type::ARRAY);
  for (const auto& conn: conns)
  {
    auto& connj = connectionsj.add(Value::Type::OBJECT);
    connj.put("element", conn.element->get_id());
    auto& imodule = conn.element->get_module();
    connj.put("input", imodule.get_input_id(*conn.element, *conn.input));
  }
}

void GetVisitor::visit_graph_input_or_output(const Dataflow::Graph&,
                                             const string&,
                                             const Dataflow::Path&,
                                             unsigned)
{
}

}} // namespaces
