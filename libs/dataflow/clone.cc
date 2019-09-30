//==========================================================================
// ViClone dataflow machines: clone.cc
//
// Clone implementation
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-dataflow.h"
#include "ot-log.h"

namespace ViGraph { namespace Dataflow {

//------------------------------------------------------------------------
// Add an element to the clone (testing)
void Clone::set_number(unsigned number)
{
  if (clones.size() == number)
    return;

  if (clones.size() > number)
  {
    clones.resize(number);
    return;
  }

  while (clones.size() < number)
    clones.emplace_back(new Graph{});

  // !!! TODO: replicated structure to new clones
}

//------------------------------------------------------------------------
// Add an element to the clone (testing)
void Clone::add(GraphElement *el)
{
  if (clones.empty())
    return;
  bool first = true;
  for (auto& graph: clones)
  {
    if (first)
      graph->add(el);
    first = false;
  }
}

//--------------------------------------------------------------------------
// Connect an element
bool Clone::connect(const string& out_name,
                    GraphElement& b, const string& in_name)
{
  if (clones.empty())
    return false;
  for (auto& graph: clones)
    if (!graph->connect(out_name, b, in_name))
      return false;
  return true;
}

//--------------------------------------------------------------------------
// Notify of a connection
void Clone::notify_connection(const string& /*in_name*/,
                              GraphElement& /*a*/, const string& /*out_name*/)
{
  throw(runtime_error("Unimplemented"));
}

//--------------------------------------------------------------------------
// Clone
Clone *Clone::clone() const
{
  auto c = new Clone{CloneModule{}};
  for (const auto& graph: clones)
  {
    c->clones.emplace_back(graph->clone());
  }
  return c;
}

//--------------------------------------------------------------------------
// Get all elements (for inspection)
const map<string, shared_ptr<GraphElement>>& Clone::get_elements() const
{
  if (clones.empty())
  {
    static const auto empty = map<string, shared_ptr<GraphElement>>{};
    return empty;
  }
  return clones.front()->get_elements();
}

//--------------------------------------------------------------------------
// Add input pin
void Clone::add_input_pin(const string& id,
                          const string& element, const string& input)
{
  if (clones.empty())
    return;
  for (auto& graph: clones)
    graph->add_input_pin(id, element, input);
}

//--------------------------------------------------------------------------
// Add output pin
void Clone::add_output_pin(const string& id,
                          const string& element, const string& output)
{
  if (clones.empty())
    return;
  for (auto& graph: clones)
    graph->add_input_pin(id, element, output);
}

//--------------------------------------------------------------------------
// Final setup for elements and calculate topology
void Clone::setup()
{
  for (const auto& graph: clones)
    graph->setup();
}

//--------------------------------------------------------------------------
// Set sample rate
void Clone::set_sample_rate(double sr)
{
  for (const auto& graph: clones)
    graph->set_sample_rate(sr);
}

//--------------------------------------------------------------------------
// Get a particular element by ID
GraphElement *Clone::get_element(const string& id)
{
  if (clones.empty())
    return nullptr;
  return clones.front()->get_element(id);
}

//--------------------------------------------------------------------------
// Clear all elements
void Clone::clear_elements()
{
  for (const auto& graph: clones)
    graph->clear_elements();
}

//--------------------------------------------------------------------------
// Prepare for a tick
void Clone::reset()
{
  for (const auto& graph: clones)
    graph->reset();
}

//--------------------------------------------------------------------------
// Collect list of all elements
void Clone::collect_elements(list<Element *>& els)
{
  for (auto& graph: clones)
    graph->collect_elements(els);
}

//--------------------------------------------------------------------------
// Accept visitors
void Clone::accept(ReadVisitor& visitor,
                   const Path& path, unsigned path_index) const
{
  visitor.visit(*this, path, path_index);
  if (!clones.empty())
    clones.front()->accept(visitor, path, path_index);
}

void Clone::accept(WriteVisitor& visitor,
                   const Path& path, unsigned path_index)
{
  visitor.visit(*this, path, path_index);
  for (auto& graph: clones)
    graph->accept(visitor, path, path_index);
}

//--------------------------------------------------------------------------
// Shutdown all elements
void Clone::shutdown()
{
  for (auto& graph: clones)
    graph->shutdown();
}

}} // namespaces
