//==========================================================================
// ViGraph JSON: get.cc
//
// JSON get functions
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-json.h"

namespace ViGraph { namespace JSON {

class GetVisitor: public Dataflow::ReadVisitor
{
private:
  Value& json;
  const Dataflow::Graph *graph = nullptr;
  const Dataflow::GraphElement *element = nullptr;
public:
  GetVisitor(Value& _json, const Dataflow::Graph *_graph):
    json{_json}, graph{_graph}
  {}
  GetVisitor(Value& _json, const Dataflow::Graph *_graph,
             const Dataflow::GraphElement *_element):
    json{_json}, graph{_graph}, element{_element}
  {}
  void visit(const Dataflow::Engine& engine) override;
  void visit(const Dataflow::Graph& graph) override;
  void visit(const Dataflow::Clone& clone) override;
  void visit(const Dataflow::Element& element) override;
  void visit(const Dataflow::SettingMember& setting) override;
  void visit(const Dataflow::InputMember& input) override;
  void visit(const Dataflow::OutputMember& output) override;
};

void get(const Dataflow::Engine& engine, Value& json,
         const Dataflow::Path& path)
{
  auto lock = engine.get_read_lock();

  auto acceptors = engine.get_visitor_acceptors(path, 0);
  for (auto& a: acceptors)
  {
    auto visitor = GetVisitor{json, a.graph ,a.element};
    a.acceptor->accept(visitor);
    return;
  }
}

void GetVisitor::visit(const Dataflow::Engine&)
{
}

void GetVisitor::visit(const Dataflow::Graph& graph)
{
  auto& module = graph.get_module();
  json.put("type", module.get_full_type());
  json.set("dynamic", module.is_dynamic() ? Value::Type::TRUE_
                                          : Value::Type::FALSE_);

  const auto& elements = graph.get_elements();
  if (!elements.empty())
  {
    auto& elementsj = json.put("elements", Value::Type::OBJECT);
    for (const auto& e: elements)
    {
      const auto& id = e.first;
      const auto& element = e.second;
      auto &elementj = elementsj.put(id, Value::Type::OBJECT);
      auto visitor = GetVisitor{elementj, &graph};
      element->accept(visitor);
    }
  }

  if (module.has_inputs())
  {
    auto& inputsj = json.put("inputs", Value::Type::OBJECT);
    module.for_each_input([&]
                          (const string& id, const Dataflow::InputMember& im)
    {
      auto& inputj = inputsj.put(id, Value::Type::OBJECT);
      auto visitor = GetVisitor{inputj, this->graph, &graph};
      im.accept(visitor);
    });
  }

  if (module.has_outputs())
  {
    auto& outputsj = json.put("outputs", Value::Type::OBJECT);
    module.for_each_output([&]
                           (const string& id, const Dataflow::OutputMember& om)
    {
      auto& outputj = outputsj.put(id, Value::Type::OBJECT);
      auto visitor = GetVisitor{outputj, this->graph, &graph};
      om.accept(visitor);
    });
  }
}

void GetVisitor::visit(const Dataflow::Clone& clone)
{
  const auto& graphs = clone.get_graphs();
  if (!graphs.empty())
    graphs.front()->accept(*this);

  const auto& module = Dataflow::clone_module;
  json.set("type", module.get_full_type());
  json.set("dynamic", module.is_dynamic() ? Value::Type::TRUE_
                                          : Value::Type::FALSE_);

  if (module.has_settings())
  {
    auto& settingsj = json.put("settings", Value::Type::OBJECT);
    module.for_each_setting([&]
                            (const string& id,
                             const Dataflow::SettingMember& sm)
    {
      auto& settingj = settingsj.put(id, Value::Type::OBJECT);
      auto visitor = GetVisitor{settingj, graph, &clone};
      sm.accept(visitor);
    });
  }
}

void GetVisitor::visit(const Dataflow::Element& element)
{
  const auto& module = element.get_module();
  json.put("type", module.get_full_type());
  json.set("dynamic", module.is_dynamic() ? Value::Type::TRUE_
                                          : Value::Type::FALSE_);

  if (module.has_settings())
  {
    auto& settingsj = json.put("settings", Value::Type::OBJECT);
    module.for_each_setting([&]
                            (const string& id,
                             const Dataflow::SettingMember& sm)
    {
      auto& settingj = settingsj.put(id, Value::Type::OBJECT);
      auto visitor = GetVisitor{settingj, graph, &element};
      sm.accept(visitor);
    });
  }
  if (module.has_inputs())
  {
    auto& inputsj = json.put("inputs", Value::Type::OBJECT);
    module.for_each_input([&]
                          (const string& id, const Dataflow::InputMember& im)
    {
      auto& inputj = inputsj.put(id, Value::Type::OBJECT);
      auto visitor = GetVisitor{inputj, graph, &element};
      im.accept(visitor);
    });
  }
  if (module.has_outputs())
  {
    auto& outputsj = json.put("outputs", Value::Type::OBJECT);
    module.for_each_output([&]
                           (const string& id, const Dataflow::OutputMember& om)
    {
      auto& outputj = outputsj.put(id, Value::Type::OBJECT);
      auto visitor = GetVisitor{outputj, graph, &element};
      om.accept(visitor);
    });
  }
}

void GetVisitor::visit(const Dataflow::SettingMember& setting)
{
  json.set("type", setting.get_type());
  json.set("value", setting.get_json(*element));
}

void GetVisitor::visit(const Dataflow::InputMember& input)
{
  json.set("type", input.get_type());
  json.set("value", input.get_json(*element));
  json.put("sample_rate", input.get_sample_rate(*element));
}

void GetVisitor::visit(const Dataflow::OutputMember& output)
{
  json.put("type", output.get_type());
  json.put("sample_rate", output.get_sample_rate(*element));
  if (graph && graph->is_output_pin(element->get_id()))
    return;
  const auto& op = output.get(*element);
  const auto& conns = op.get_connections();
  if (conns.empty())
    return;
  auto& connectionsj = json.put("connections", Value::Type::ARRAY);
  for (const auto& conn: conns)
  {
    const auto& imodule = conn.element->get_module();
    const auto& input_id = imodule.get_input_id(*conn.element, *conn.input);
    // Input id will be empty in the case where the connection is to a clone
    // and is not to the first copy
    if (!input_id.empty())
    {
      auto& connj = connectionsj.add(Value::Type::OBJECT);
      connj.put("element", conn.element->get_id());
      connj.put("input", input_id);
    }
  }
}

}} // namespaces
