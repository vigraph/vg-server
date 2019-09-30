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
  elements[el->id].reset(el);
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
  return output_pin->connect(pin_info.connection, b, in_name);
}

//--------------------------------------------------------------------------
// Notify of a connection
void Graph::notify_connection(const string& /*in_name*/,
                              GraphElement& /*a*/, const string& /*out_name*/)
{
  throw(runtime_error("Unimplemented"));
}

//--------------------------------------------------------------------------
// Clone
Graph *Graph::clone() const
{
  auto g = new Graph{GraphModule{}};
  for (const auto& el: elements)
  {
    g->elements.emplace(el.first, el.second->clone());
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
void Graph::accept(ReadVisitor& visitor,
                   const Path& path, unsigned path_index) const
{
  visitor.visit(*this, path, path_index);
  for (auto& eit: elements)
  {
    auto sv = visitor.getSubElementVisitor(eit.first);
    if (sv)
      eit.second->accept(*sv, path, path_index);
  }
}

void Graph::accept(WriteVisitor& visitor,
                   const Path& path, unsigned path_index)
{
  visitor.visit(*this, path, path_index);
  for (auto& eit: elements)
  {
    auto sv = visitor.getSubElementVisitor(eit.first);
    if (sv)
      eit.second->accept(*sv, path, path_index);
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
