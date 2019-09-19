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
  return make_unique<GetVisitor>(json);
}

void GetVisitor::visit(Dataflow::Graph& graph)
{
  auto& module = graph.get_module();
  json.put("type", module.get_full_type());
  if (module.has_inputs())
  {
    auto& inputsj = json.put("inputs", Value::Type::OBJECT);
    module.for_each_input([&inputsj](const string& id,
                                 const Dataflow::Module::InputMember& input)
    {
      auto& inputj = inputsj.put(id, Value::Type::OBJECT);
      inputj.put("type", input.get_type());
    });
  }
  if (module.has_outputs())
  {
    auto& outputsj = json.put("outputs", Value::Type::OBJECT);
    module.for_each_output([&graph, &outputsj, this]
              (const string& id, const Dataflow::Module::OutputMember& output)
    {
      auto& outputj = outputsj.put(id, Value::Type::OBJECT);
      outputj.put("type", output.get_type());
      this->output_pins.insert(id);
      const auto& op = output.get(graph);
      const auto& conns = op.get_connections();
      if (conns.empty())
        return;
      auto& connectionsj = outputj.put("connections", Value::Type::ARRAY);
      for (const auto& conn: conns)
      {
        auto& connj = connectionsj.add(Value::Type::OBJECT);
        connj.put("element", conn.element->id);
        auto& imodule = conn.element->get_module();
        connj.put("input", imodule.get_input_id(*conn.element, *conn.input));
      }
    });
  }
}

unique_ptr<Dataflow::Visitor> GetVisitor::getSubElementVisitor(const string& id)
{
  auto eit = json.o.find("elements");
  if (eit == json.o.end())
    eit = json.o.emplace("elements", Value::Type::OBJECT).first;
  auto& elements = eit->second;
  auto no_connections = output_pins.find(id) != output_pins.end();
  return make_unique<GetVisitor>(elements.put(id, Value::Type::OBJECT),
                                 no_connections);
}

void GetVisitor::visit(Dataflow::Element& element)
{
  auto& module = element.get_module();
  json.put("type", module.get_full_type());

  if (module.has_settings())
  {
    auto& settingsj = json.put("settings", Value::Type::OBJECT);
    module.for_each_setting([&element, &settingsj](const string& id,
                             const Dataflow::Module::SettingMember& setting)
    {
      auto& settingj = settingsj.put(id, Value::Type::OBJECT);
      settingj.put("value", setting.get_json(element));
    });
  }

  if (module.has_inputs())
  {
    auto& inputsj = json.put("inputs", Value::Type::OBJECT);
    module.for_each_input([&element, &inputsj](const string& id,
                                 const Dataflow::Module::InputMember& input)
    {
      auto& inputj = inputsj.put(id, Value::Type::OBJECT);
      inputj.put("value", input.get_json(element));
    });
  }

  if (!no_connections && module.has_outputs())
  {
    auto& outputsj = json.put("outputs", Value::Type::OBJECT);
    module.for_each_output([&element, &outputsj](const string& id,
                               const Dataflow::Module::OutputMember& output)
    {
      const auto& op = output.get(element);
      const auto& conns = op.get_connections();
      if (conns.empty())
        return;
      auto& outputj = outputsj.put(id, Value::Type::OBJECT);
      auto& connectionsj = outputj.put("connections", Value::Type::ARRAY);
      for (const auto& conn: conns)
      {
        auto& connj = connectionsj.add(Value::Type::OBJECT);
        connj.put("element", conn.element->id);
        auto& imodule = conn.element->get_module();
        connj.put("input", imodule.get_input_id(*conn.element, *conn.input));
      }
    });
  }
}

}} // namespaces
