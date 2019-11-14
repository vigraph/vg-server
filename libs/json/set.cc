//==========================================================================
// ViGraph JSON: set.cc
//
// JSON set functions
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-json.h"

namespace ViGraph { namespace JSON {

class SetVisitor: public Dataflow::WriteVisitor
{
public:
  enum class Phase
  {
    setup,
    connection
  };
private:
  Dataflow::Engine& engine;
  const Value& json;
  Phase phase = Phase::setup;
  const string id;
  Dataflow::Graph *graph = nullptr;
  Dataflow::Clone *clone = nullptr;
  Dataflow::GraphElement *element = nullptr;

public:
  // Top level
  SetVisitor(Dataflow::Engine& _engine, const Value& _json, Phase _phase,
             const string& _id):
    engine{_engine}, json{_json}, phase{_phase}, id{_id}
  {}
  // Element visitor
  SetVisitor(Dataflow::Engine& _engine, const Value& _json,
             Phase _phase, const string& _id, Dataflow::Graph *_graph,
             Dataflow::Clone *_clone):
    engine{_engine}, json{_json}, phase{_phase}, id{_id}, graph{_graph},
    clone{_clone}
  {}
  // Attribute visitor
  SetVisitor(Dataflow::Engine& _engine, const Value& _json,
             Phase _phase, const string& _id,
             Dataflow::GraphElement *_element, Dataflow::Graph *_graph):
    engine{_engine}, json{_json}, phase{_phase}, id{_id},
    graph{_graph}, element{_element}
  {}
  void visit(Dataflow::Engine& engine) override;
  void visit(Dataflow::Graph& graph) override;
  void visit(Dataflow::Clone& clone) override;
  void visit(Dataflow::Element& element) override;
  void visit(const Dataflow::SettingMember& setting) override;
  void visit(const Dataflow::InputMember& input) override;
  void visit(const Dataflow::OutputMember& output) override;
};

Dataflow::GraphElement *create_element(
                    Dataflow::Engine& engine, Dataflow::Graph& graph,
                    Dataflow::Clone *clone,
                    const string& id, const Value& json)
{
  if (id.empty())
    throw runtime_error("Graph element requires an 'id'");
  const auto &typej = json.get("type");
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

    auto visitor = SetVisitor{engine, json, SetVisitor::Phase::setup,
                              id, &graph, clone};
    element->accept(visitor);
    return element;
  }
  else
  {
    throw(runtime_error{"Unknown element type: " + type});
  }
}

void set(Dataflow::Engine& engine, const Value& json,
         const Dataflow::Path& path)
{
  auto lock = engine.get_write_lock();

  auto acceptors = engine.get_visitor_acceptors(path, 0);
  if (acceptors.empty())
    throw runtime_error("Path not found");

  for (auto& a: acceptors)
  {
    // Setup
    if (a.create)
    {
      if (a.attribute)
      {
        // Graph input/output
        const auto& dir = json["direction"].as_str();
        auto graph = dynamic_cast<Dataflow::Graph *>(a.element);
        if (graph)
        {
          if (dir == "in")
            graph->add_input_pin(a.id, a.id, "input");
          else if (dir == "out")
            graph->add_output_pin(a.id, a.id, "output");
          else
            throw(runtime_error{"Direction is required"});
        }
        else
        {
          throw(runtime_error{"Path not found"});
        }
        continue;
      }
      else
      {
        a.acceptor = create_element(engine, *a.graph, a.clone, a.id, json);
      }
    }

    if (a.acceptor)
    {
      auto visitor = SetVisitor{engine, json, SetVisitor::Phase::setup,
                                a.id, a.graph, a.clone};
      a.acceptor->accept(visitor);
    }
    else
    {
      auto visitor = SetVisitor{engine, json, SetVisitor::Phase::setup,
                                a.id, a.element, a.graph};
      a.member_acceptor->accept(visitor);
    }
    if (a.setting)
      a.element->setup();

    // Connection
    if (a.acceptor)
    {
      auto visitor = SetVisitor{engine, json, SetVisitor::Phase::connection,
                                a.id, a.graph, a.clone};
      a.acceptor->accept(visitor);
    }
    else
    {
      auto visitor = SetVisitor{engine, json, SetVisitor::Phase::connection,
                                a.id, a.element, a.graph};
      a.member_acceptor->accept(visitor);
    }
  }

  engine.update_elements();
}

void SetVisitor::visit(Dataflow::Engine& engine)
{
  engine.get_graph().accept(*this);
}

void SetVisitor::visit(Dataflow::Graph& graph)
{
  switch (phase)
  {
    case Phase::setup:
      {
        graph.shutdown();

        // Contained elements
        auto& elementsj = json.get("elements");
        if (!!elementsj && elementsj.type == Value::Type::OBJECT)
        {
          for (const auto& it: elementsj.o)
          {
            const auto& id = it.first;
            const auto& elementj = it.second;

            create_element(engine, graph, clone, id, elementj);
          }
        }

        // Graph Inputs
        auto& inputsj = json.get("inputs");
        if (!!inputsj && inputsj.type == Value::Type::OBJECT)
        {
          for (const auto& iit: inputsj.o)
          {
            const auto& iid = iit.first;
            if (iid.empty())
              throw runtime_error("Graph input pin requires an 'id'");

            graph.add_input_pin(iid, iid, "input");
          }
        }

        // Graph Outputs
        auto& outputsj = json.get("outputs");
        if (!!outputsj && outputsj.type == Value::Type::OBJECT)
        {
          for (const auto& oit: outputsj.o)
          {
            const auto& oid = oit.first;
            if (oid.empty())
              throw runtime_error("Graph output pin requires an 'id'");

            graph.add_output_pin(oid, oid, "output");
          }
        }
      }
      break;

    case Phase::connection:
      {
        // Connect sub-elements
        auto& elementsj = json.get("elements");
        if (!!elementsj && elementsj.type == Value::Type::OBJECT)
        {
          for (const auto& it: elementsj.o)
          {
            const auto& id = it.first;
            const auto& elementj = it.second;
            auto element = graph.get_element(id);
            if (element)
            {
              auto visitor = SetVisitor{engine, elementj, phase, id,
                                        &graph, clone};
              element->accept(visitor);
            }
          }
        }
        // Connect graph's outputs
        const auto& module = graph.get_module();
        const auto& outputsj = json.get("outputs");
        if (!!outputsj && outputsj.type == Value::Type::OBJECT)
        {
          for (const auto& oit: outputsj.o)
          {
            const auto& id = oit.first;
            const auto o = module.get_output(id);
            if (o)
            {
              const auto& oj = oit.second;
              auto visitor = SetVisitor{engine, oj, phase, id,
                                        &graph, this->graph};
              o->accept(visitor);
            }
          }
        }
      }
      break;
  }
}

void SetVisitor::visit(Dataflow::Clone& clone)
{
  switch (phase)
  {
    case Phase::setup:
      {
        clone.shutdown();
        // Settings
        const auto& module = Dataflow::clone_module;
        auto& settingsj = json.get("settings");
        if (!!settingsj && settingsj.type == Value::Type::OBJECT)
        {
          for (const auto& sit: settingsj.o)
          {
            const auto& sid = sit.first;
            auto s = module.get_setting(sid);
            if (s)
            {
              auto visitor = SetVisitor{engine, sit.second, phase, id, &clone,
                                        graph};
              s->accept(visitor);
            }
            else
            {
              Log::Error elog;
              elog << clone.get_id() << ": Unknown setting '"
                   << sid << "'" << endl;
            }
          }
        }
        clone.setup();
      }
      // Fall through

    case Phase::connection:
      {
        const auto& graphs = clone.get_graphs();
        auto visitor = SetVisitor{engine, json, phase, id, graph, &clone};
        for (auto& g: graphs)
          g->accept(visitor);
      }
      break;
  }
}

void SetVisitor::visit(Dataflow::Element& element)
{
  switch (phase)
  {
    case Phase::setup:
      {
        const auto& module = element.get_module();
        // Settings
        auto& settingsj = json.get("settings");
        if (!!settingsj && settingsj.type == Value::Type::OBJECT)
        {
          for (const auto& sit: settingsj.o)
          {
            const auto& sid = sit.first;
            const auto s = module.get_setting(sid);
            if (s)
            {
              auto visitor = SetVisitor{engine, sit.second, phase, id,
                                        &element, graph};
              s->accept(visitor);
            }
            else
            {
              // Try inputs
              const auto i = module.get_input(sid);
              if (i)
              {
                auto visitor = SetVisitor{engine, sit.second, phase, id,
                                          &element, graph};
                i->accept(visitor);
              }
              else
              {
                Log::Error elog;
                elog << element.get_id() << ": Unknown setting '"
                     << sid << "'" << endl;
              }
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
            const auto i = module.get_input(iid);
            if (i)
            {
              auto visitor = SetVisitor{engine, iit.second, phase, id,
                                        &element, graph};
              i->accept(visitor);
            }
            else
            {
              Log::Error elog;
              elog << element.get_id() << ": Unknown input '"
                   << iid << "'" << endl;
            }
          }
        }
        element.setup();
      }
      break;
    case Phase::connection:
      {
        // Connect graph's outputs
        const auto& module = element.get_module();
        const auto& outputsj = json.get("outputs");
        if (!!outputsj && outputsj.type == Value::Type::OBJECT)
        {
          for (const auto& oit: outputsj.o)
          {
            const auto& id = oit.first;
            const auto o = module.get_output(id);
            if (o)
            {
              const auto& oj = oit.second;
              auto visitor = SetVisitor{engine, oj, phase, id, &element, graph};
              o->accept(visitor);
            }
          }
        }
      }
      break;
  }
}

void SetVisitor::visit(const Dataflow::SettingMember& setting)
{
  const auto& valuej = json.get("value");
  if (!valuej)
    return;
  setting.set_json(*element, valuej);
}

void SetVisitor::visit(const Dataflow::InputMember& input)
{
  const auto& valuej = json.get("value");
  if (!valuej)
    return;
  input.set_json(*element, valuej);
}

void SetVisitor::visit(const Dataflow::OutputMember& output)
{
  if (!graph || phase != Phase::connection)
    return; // Can't make connections without a scope

  const auto& connectionsj = json.get("connections");
  if (!connectionsj || connectionsj.type != Value::Type::ARRAY)
    return;

  // Disconnect any existing connections
  auto& op = output.get(*element);
  op.disconnect();

  for (const auto& connectionj: connectionsj.a)
  {
    const auto& iid = connectionj["element"].as_str();
    const auto& iinput = connectionj["input"].as_str();
    auto ielement = graph->get_element(iid);
    if (!ielement && iid == graph->get_id())
      ielement = graph;
    if (!ielement)
    {
      Log::Error log;
      log << "Unknown element '" << iid << "' for element '"
          << element->get_id() << "' output connection on '" << id
          << "' in scope of '" << graph->get_id() << "'"
          << endl;
      continue;
    }

    if (!element->connect(id, *ielement, iinput))
    {
      Log::Error log;
      log << "Could not connect " << element->get_id() << "." << id
          << " to " << ielement->get_id() << "." << iinput << endl;
      continue;
    }
  }
}

}} // namespaces
