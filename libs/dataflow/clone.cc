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
// Set id
void Clone::set_id(const string& id)
{
  GraphElement::set_id(id);
  for (auto& clone: clones)
    clone.graph->set_id(id);
}

//--------------------------------------------------------------------------
// Get module
const Module& Clone::get_module() const
{
  if (clones.empty())
    return module;
  return clones.front().graph->get_module();
}

//------------------------------------------------------------------------
// Set number of clones
void Clone::set_number(unsigned number)
{
  if (number < 1)
    return;

  if (clones.size() == number)
    return;

  if (clones.size() > number)
  {
    clones.resize(number);
  }

  while (clones.size() < number)
  {
    clones.emplace_back(clones.front().graph->clone());
    // !!! TODO: find clone infos
  }

  // Sort out clone info numbering
  auto i = 0;
  for (auto& graph: clones)
  {
    for (auto info: graph.infos)
    {
      info->clone_number = i + 1;
      info->clone_total = clones.size();
    };
    ++i;
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
  {
    if (!graph.graph->connect(out_name, b, in_name))
      return false;
    b.notify_connection(in_name, *graph.graph, out_name);
  }
  return true;
}

//--------------------------------------------------------------------------
// Notify of a connection
void Clone::notify_connection(const string& /*in_name*/,
                              GraphElement& /*a*/, const string& /*out_name*/)
{
  //throw(runtime_error("Unimplemented"));
}

//--------------------------------------------------------------------------
// Clone
Clone *Clone::clone() const
{
  auto c = new Clone{clone_module};
  for (const auto& graph: clones)
  {
    c->clones.emplace_back(graph.graph->clone());
    // !!! TODO: sort out clone infos
  }
  return c;
}

//--------------------------------------------------------------------------
// Final setup for elements and calculate topology
void Clone::setup()
{
  for (const auto& graph: clones)
    graph.graph->setup();
}

//--------------------------------------------------------------------------
// Prepare for a tick
void Clone::reset()
{
  for (const auto& graph: clones)
    graph.graph->reset();
}

//--------------------------------------------------------------------------
// Collect list of all elements
void Clone::collect_elements(list<Element *>& els)
{
  for (auto& graph: clones)
    graph.graph->collect_elements(els);
}

//--------------------------------------------------------------------------
// Accept visitors
void Clone::accept(ReadVisitor& visitor,
                   const Path& path, unsigned path_index) const
{
  if (!clones.empty())
    clones.front().graph->accept(visitor, path, path_index);
  visitor.visit(*this, path, path_index);
}

void Clone::accept(WriteVisitor& visitor,
                   const Path& path, unsigned path_index)
{
  visitor.visit(*this, path, path_index);
  for (auto& graph: clones)
    graph.graph->accept(visitor, path, path_index);
}

//--------------------------------------------------------------------------
// Shutdown all elements
void Clone::shutdown()
{
  for (auto& graph: clones)
    graph.graph->shutdown();
}

//==========================================================================
// Clone Info

//--------------------------------------------------------------------------
// Tick
void CloneInfo::tick(const TickData& td)
{
  sample_iterate(td.nsamples, {}, {},
                 tie(number, total, fraction),
                 [&](double& number, double& total, double& fraction)
  {
    number = clone_number;
    total = clone_total;
    fraction = clone_total ? (clone_number - 1) / clone_total : 0;
  });

}

}} // namespaces
