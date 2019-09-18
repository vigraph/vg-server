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
  if (oit != output_pins.end())
  {
    auto& output_pin = oit->second;
    return output_pin->connect("output", b, in_name);
  }
  // Now check input pins (connecting to internal stuff)
  auto iit = input_pins.find(out_name);
  if (iit == input_pins.end())
    return false;

  auto& input_pin = iit->second;
  return input_pin->connect("output", b, in_name);
}

//--------------------------------------------------------------------------
// Notify of a connection
void Graph::notify_connection(const string& /*in_name*/,
                              GraphElement& /*a*/, const string& /*out_name*/)
{
  throw(runtime_error("Unimplemented"));
}

//--------------------------------------------------------------------------
// Add input pin
void Graph::add_input_pin(const string& id, shared_ptr<GraphElement> pin)
{
  input_pins.emplace(id, pin);
  module.inputs.emplace(id, *pin);
  module.outputs.emplace(id, *pin);
}

//--------------------------------------------------------------------------
// Add output pin
void Graph::add_output_pin(const string& id, shared_ptr<GraphElement> pin)
{
  output_pins.emplace(id, pin);
  module.inputs.emplace(id, *pin);
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
// Accept a visitor
void Graph::accept(Visitor& visitor)
{
  visitor.visit(*this);
  for (auto& eit: elements)
  {
    auto sv = visitor.getSubElementVisitor(eit.first);
    if (sv)
      eit.second->accept(*sv);
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
