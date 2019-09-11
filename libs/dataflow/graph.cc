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
bool Graph::connect(const string& /*out_name*/,
                    GraphElement& /*b*/, const string &/*in_name*/)
{
  throw(runtime_error("Unimplemented"));
  return true;
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

//==========================================================================
// Module bits
ElementSetting& GraphModule::GraphSettingMember::get(GraphElement& ) const
{
  throw(runtime_error("unimplemented"));
}

JSON::Value GraphModule::GraphSettingMember::get_json(GraphElement& ) const
{
  return {};
}

void GraphModule::GraphSettingMember::set_json(GraphElement& ,
                                               const JSON::Value&) const
{
}

ElementInput& GraphModule::GraphInputMember::get(GraphElement& ) const
{
  throw(runtime_error("unimplemented"));
}

JSON::Value GraphModule::GraphInputMember::get_json(GraphElement& ) const
{
  return {};
}

void GraphModule::GraphInputMember::set_json(GraphElement& ,
                                             const JSON::Value&) const
{
}

ElementOutput& GraphModule::GraphOutputMember::get(GraphElement& ) const
{
  throw(runtime_error("unimplemented"));
}

}} // namespaces
