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
void Graph::add(Element *el)
{
  elements[el->id].reset(el);
}

//------------------------------------------------------------------------
// Create an element
Element *Graph::create_element(const string& type, const string& id)
{
  const auto el = engine.create(type);
  if (!el) throw runtime_error("No such dataflow element " + type);
  el->id = id;

  // Point back to us
  el->graph = this;

  return el;
}

//------------------------------------------------------------------------
// Add an element
void Graph::add_element(const string& type, const string& id)
{
  auto el = create_element(type, id);
  if (el)
    add(el);
}

//------------------------------------------------------------------------
// Final setup for elements and calculate topology
void Graph::setup()
{
  for(const auto& it: elements)
    it.second->setup();
}

//------------------------------------------------------------------------
// Tick all elements in topological order
void Graph::tick(const TickData& td)
{
  MT::RWReadLock lock(mutex);
  auto to_tick = list<Element *>{};
  for (auto& el: elements)
    to_tick.push_back(el.second.get());
  while (!to_tick.empty())
  {
    for (auto it = to_tick.begin(); it != to_tick.end();)
    {
      if ((*it)->ready())
      {
        (*it)->tick(td);
        (*it)->reset();
        it = to_tick.erase(it);
      }
      else
      {
        ++it;
      }
    }
  }
}

//------------------------------------------------------------------------
// Get a particular element by ID
Element *Graph::get_element(const string& id)
{
  MT::RWReadLock lock(mutex);
  if (elements.find(id) != elements.end())
    return elements[id].get();
  else
    return nullptr;
}

//------------------------------------------------------------------------
// Get the nearest particular element by section and type, looking upwards
// in ancestors
shared_ptr<Element> Graph::get_nearest_element(const string& section,
                                               const string& type)
{
  MT::RWReadLock lock(mutex);
  // !!! Linear search!
  for(const auto& it: elements)
    if (it.second->module.get_section() == section &&
        it.second->module.get_id() == type)
      return it.second;

  if (parent)
    return parent->get_nearest_element(section, type);
  else
    return nullptr;
}

//------------------------------------------------------------------------
// Delete an item (from REST)
// path is a path/to/leaf
void Graph::delete_item(const string& path)
{
  if (path.empty())
    throw runtime_error("Can't delete in whole graph");

  vector<string> bits = Text::split(path, '/', false, 2);
  const auto& it = elements.find(bits[0]);
  if (it == elements.end())
    throw runtime_error("No such element "+bits[0]+" in graph");
  auto el = it->second.get();

  if (bits.size() > 1)
  {
    // Pass down to subgraph/element
    el->delete_item(bits[1]);
  }
  else
  {
    elements.erase(it);
  }
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
