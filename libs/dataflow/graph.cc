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

//------------------------------------------------------------------------
// Add an element to the graph (testing)
void Graph::add(GraphElement *el)
{
  elements[el->get_id()].reset(el);
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
  for (const auto& el: elements)
  {
    g->elements.emplace(el.first, el.second->clone());
  }

  // !!! TODO: clone element & graph connections
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
template<class G, class V>
inline void accept_visitor(G& graph, V& visitor,
                           const Path& path, unsigned path_index)
{
  if (path.reached(path_index))
  {
    visitor.visit(graph, path, path_index);

    auto& module = graph.get_module();
    if (module.has_inputs())
    {
      module.for_each_input([&graph, &visitor, &path, &path_index]
                            (const string& id,
                             const InputMember& input)
      {
        auto iv = visitor.get_element_input_visitor(id);
        if (iv)
          input.accept(*iv, path, ++path_index, graph);
      });
    }

    if (module.has_outputs())
    {
      module.for_each_output([&graph, &visitor, &path, &path_index]
                             (const string& id,
                              const OutputMember& output)
      {
        auto ov = visitor.get_element_output_visitor(id);
        if (ov)
          output.accept(*ov, path, ++path_index, graph);
      });
    }

    auto& elements = graph.get_elements();
    for (auto& eit: elements)
    {
      auto sv = visitor.get_sub_element_visitor(eit.first, graph);
      if (sv)
        eit.second->accept(*sv, path, ++path_index);
    }
  }
  else
  {
    auto part = path.get(path_index);

    switch (part.type)
    {
      case Path::PartType::attribute:
        break;
      case Path::PartType::element:
        {
          auto& elements = graph.get_elements();
          auto eit = elements.find(part.name);
          if (eit == elements.end())
            throw(runtime_error{"Element not found: " + part.name});
          eit->second->accept(visitor, path, ++path_index);
         }
        break;
      default:
        break;
    }
  }
}

void Graph::accept(ReadVisitor& visitor,
                   const Path& path, unsigned path_index) const
{
  accept_visitor(*this, visitor, path, path_index);
}

void Graph::accept(WriteVisitor& visitor,
                   const Path& path, unsigned path_index)
{
  accept_visitor(*this, visitor, path, path_index);
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
