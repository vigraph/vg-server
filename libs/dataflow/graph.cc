//==========================================================================
// ViGraph dataflow machines: graph.cc
//
// Graph structure implementation
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-dataflow.h"
#include "ot-log.h"

namespace ViGraph { namespace Dataflow {

//--------------------------------------------------------------------------
// Add an element to the graph (testing)
void Graph::add(GraphElement *el)
{
  elements[el->get_id()].reset(el);
}

//--------------------------------------------------------------------------
// Remove an element
void Graph::remove(const string& id)
{
  remove_input_pin(id);
  remove_output_pin(id);
  auto it = elements.find(id);
  if (it == elements.end())
    throw(runtime_error{"Element not found: " + id});
  it->second->shutdown();
  elements.erase(it);
}


//--------------------------------------------------------------------------
// Connect an element
bool Graph::connect(const string& out_name,
                    GraphElement& b, const string& in_name)
{
  // Check if connecting an output pin
  auto oit = output_pins.find(out_name);
  if (oit == output_pins.end())
    return false;

  const auto& pin_info = oit->second;
  auto output_pin = get_element(pin_info.element);
  if (!output_pin)
    return false;

  // Pass connection on to the pin
  return output_pin->connect(pin_info.connection, b, in_name);
}

//--------------------------------------------------------------------------
// Notify of a connection
void Graph::notify_connection(const string& in_name,
                              GraphElement& a, const string& out_name)
{
  auto iit = input_pins.find(in_name);
  if (iit == input_pins.end())
    return;

  const auto& pin_info = iit->second;
  auto input_pin = get_element(pin_info.element);
  if (!input_pin)
    return;

  // Pass notification to pin
  input_pin->notify_connection("input", a, out_name);
}

//--------------------------------------------------------------------------
// Clone
Graph *Graph::clone() const
{
  auto g = new Graph{GraphModule{}};
  g->set_id(get_id());

  // Elements
  auto pairs = vector<pair<GraphElement *, GraphElement *>>{};
  for (const auto& el: elements)
  {
    auto c = el.second->clone();
    c->setup();
    g->elements.emplace(el.first, c);
    pairs.emplace_back(el.second.get(), c);
  }

  // Input pins
  for (const auto &ip: input_pins)
  {
    g->add_input_pin(ip.first, ip.second.element, ip.second.connection);
  }

  // Output pins
  for (const auto &op: output_pins)
  {
    g->add_output_pin(op.first, op.second.element, op.second.connection);
  }

  // Element connections (including outbound external)
  for (auto& els: pairs)
  {
    auto orig = els.first;
    auto clone = els.second;
    auto& m = orig->get_module();
    if (m.has_outputs())
    {
      m.for_each_output([&orig, &clone, &g]
                        (const string& id, const OutputMember& output)
      {
        auto& op = output.get(*orig);
        const auto& conns = op.get_connections();
        for (const auto& conn: conns)
        {
          const auto& eid = conn.element->get_id();
          auto e = g->get_element(eid);
          if (!e)
            continue;
          const auto& emodule = conn.element->get_module();
          const auto& einput = emodule.get_input_id(*conn.element, *conn.input);
          clone->connect(id, *e, einput);
        }
      });
    }
  }

  // Connect inputs to graph
  auto& module = get_module();
  for (const auto &ip: input_pins)
  {
    auto im = module.get_input(ip.first);
    if (im)
    {
      auto& i = im->get(*this);
      const auto conns = i.get_connections();
      for (auto& conn: conns)
      {
        const auto& emodule = conn.element->get_module();
        const auto& eoutput = emodule.get_output_id(*conn.element,
                                                    *conn.output);
        conn.element->connect(eoutput, *g, ip.first);
      }
    }
  }

  return g;
}

//--------------------------------------------------------------------------
// Add input pin
void Graph::add_input_pin(const string& id,
                          const string& element, const string& input)
{
  auto pin = get_element(element);
  if (!pin)
  {
    Log::Error log;
    log << "Bad input pin element " << element << endl;
    return;
  }
  input_pins.emplace(id, PinInfo{element, input});
  module.inputs.emplace(id, *pin);
}

//--------------------------------------------------------------------------
// Remove input pin
void Graph::remove_input_pin(const string& id)
{
  input_pins.erase(id);
  module.inputs.erase(id);
}

//--------------------------------------------------------------------------
// Add output pin
void Graph::add_output_pin(const string& id,
                          const string& element, const string& output)
{
  auto pin = get_element(element);
  if (!pin)
  {
    Log::Error log;
    log << "Bad output pin element " << element << endl;
    return;
  }
  output_pins.emplace(id, PinInfo{element, output});
  module.outputs.emplace(id, *pin);
}

//--------------------------------------------------------------------------
// Remove output pin
void Graph::remove_output_pin(const string& id)
{
  auto it = output_pins.find(id);
  if (it == output_pins.end())
    return;
  auto el = elements.find(id);
  if (el != elements.end())
  {
    auto& m = el->second->get_module();
    auto o = m.get_output(*el->second, it->second.connection);
    if (o)
      o->disconnect();
  }
  output_pins.erase(it);
  module.outputs.erase(id);
}

//------------------------------------------------------------------------
// Final setup for elements and calculate topology
void Graph::setup()
{
  for(const auto& it: elements)
    it.second->setup();
}

//------------------------------------------------------------------------
// Get a particular element by ID
GraphElement *Graph::get_element(const string& id)
{
  MT::RWReadLock lock(mutex);
  if (elements.find(id) != elements.end())
    return elements[id].get();
  else
    return nullptr;
}

//--------------------------------------------------------------------------
// Accept visitors
void Graph::accept(ReadVisitor& visitor,
                   const Path& path, unsigned path_index) const
{
  if (path.reached(path_index))
  {
    visitor.visit(*this, path, path_index);

    auto& module = get_module();
    if (module.has_inputs())
    {
      module.for_each_input([this, &visitor, &path, &path_index]
                            (const string& id,
                             const InputMember& input)
      {
        auto iv = visitor.get_element_input_visitor(*this, id,
                                                    path, path_index);
        if (iv)
          input.accept(*iv, path, path_index + 1, *this);
      });
    }

    if (module.has_outputs())
    {
      module.for_each_output([this, &visitor, &path, &path_index]
                             (const string& id,
                              const OutputMember& output)
      {
        auto ov = visitor.get_element_output_visitor(*this, id,
                                                     path, path_index);
        if (ov)
          output.accept(*ov, path, path_index + 1, *this);
      });
    }

    auto& elements = get_elements();
    for (auto& eit: elements)
    {
      auto sv = visitor.get_sub_element_visitor(*this, eit.first,
                                                path, path_index);
      if (sv)
        eit.second->accept(*sv, path, path_index + 1);
    }
  }
  else
  {
    auto part = path.get(path_index);

    switch (part.type)
    {
      case Path::PartType::attribute:
        {
          auto& module = get_module();
          auto s = module.get_setting(part.name);
          if (s)
          {
            auto sv = visitor.get_element_setting_visitor(*this, part.name,
                                                          path, path_index);
            if (sv)
              s->accept(*sv, path, path_index + 1, *this);
          }
          else
          {
            auto i = module.get_input(part.name);
            if (i)
            {
              auto iv = visitor.get_element_input_visitor(*this, part.name,
                                                          path, path_index);
              if (iv)
                i->accept(*iv, path, path_index + 1, *this);
            }
            else
            {
              auto o = module.get_output(part.name);
              if (o)
              {
                auto ov = visitor.get_element_output_visitor(*this, part.name,
                                                             path, path_index);
                if (ov)
                  o->accept(*ov, path, path_index + 1, *this);
              }
              else
              {
                visitor.visit_graph_input_or_output(*this, part.name,
                                                    path, path_index);
              }
            }
          }
        }
        break;
      case Path::PartType::element:
        {
          auto& elements = get_elements();
          auto eit = elements.find(part.name);
          auto exists = (eit != elements.end());
          if (!exists)
            throw(runtime_error{"Element not found: " + part.name});
          auto sv = visitor.get_sub_element_visitor(*this, part.name,
                                                    path, path_index);
          if (sv)
            eit->second->accept(*sv, path, ++path_index);
        }
        break;
      default:
        break;
    }
  }
}

void Graph::accept(WriteVisitor& visitor,
                   const Path& path, unsigned path_index)
{
  if (path.reached(path_index))
  {
    visitor.visit(*this, path, path_index);

    auto& module = get_module();
    if (module.has_inputs())
    {
      module.for_each_input([this, &visitor, &path, &path_index]
                            (const string& id,
                             const InputMember& input)
      {
        auto iv = visitor.get_element_input_visitor(*this, id,
                                                    path, path_index);
        if (iv)
          input.accept(*iv, path, path_index + 1, *this);
      });
    }

    if (module.has_outputs())
    {
      module.for_each_output([this, &visitor, &path, &path_index]
                             (const string& id,
                              const OutputMember& output)
      {
        auto ov = visitor.get_element_output_visitor(*this, id,
                                                     path, path_index);
        if (ov)
          output.accept(*ov, path, path_index + 1, *this);
      });
    }

    auto& elements = get_elements();
    for (auto& eit: elements)
    {
      auto sv = visitor.get_sub_element_visitor(*this, eit.first,
                                                path, path_index);
      if (sv)
        eit.second->accept(*sv, path, path_index + 1);
    }
  }
  else
  {
    auto part = path.get(path_index);

    switch (part.type)
    {
      case Path::PartType::attribute:
        {
          auto& module = get_module();
          auto s = module.get_setting(part.name);
          if (s)
          {
            auto sv = visitor.get_element_setting_visitor(*this, part.name,
                                                          path, path_index);
            if (sv)
            {
              s->accept(*sv, path, path_index + 1, *this);
              setup();
            }
          }
          else
          {
            auto i = module.get_input(part.name);
            if (i)
            {
              auto iv = visitor.get_element_input_visitor(*this, part.name,
                                                          path, path_index);
              if (iv)
                i->accept(*iv, path, path_index + 1, *this);
            }
            else
            {
              auto o = module.get_output(part.name);
              if (o)
              {
                auto ov = visitor.get_element_output_visitor(*this, part.name,
                                                             path, path_index);
                if (ov)
                  o->accept(*ov, path, path_index + 1, *this);
              }
              else
              {
                visitor.visit_graph_input_or_output(*this, part.name,
                                                    path, path_index);
              }
            }
          }
        }
        break;
      case Path::PartType::element:
        {
          auto& elements = get_elements();
          auto eit = elements.find(part.name);
          auto exists = (eit != elements.end());
          if (!exists && !path.reached(path_index + 1))
            throw(runtime_error{"Element not found: " + part.name});
          auto sv = visitor.get_sub_element_visitor(*this, part.name,
                                                    path, path_index);
          if (sv)
          {
            if (!exists)
            {
              eit = elements.find(part.name); // It may just have been created
              if (eit == elements.end())
                throw(runtime_error{"Element not found: " + part.name});
            }
            eit->second->accept(*sv, path, ++path_index);
          }
        }
        break;
      default:
        break;
    }
  }
}

//------------------------------------------------------------------------
// Shutdown all elements
void Graph::shutdown()
{
  MT::RWWriteLock lock(mutex);
  for(const auto it: elements)
    it.second->shutdown();

  // Remove all elements before modules unloaded
  elements.clear();
}

}} // namespaces
