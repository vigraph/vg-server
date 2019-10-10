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

//--------------------------------------------------------------------------
// Register a clone info
void Clone::register_info(const Graph& graph, CloneInfo *info)
{
  auto i = 0;
  for (auto& g: clones)
  {
    if (g.graph.get() == &graph)
    {
      g.infos.insert(info);
      info->clone_number = i + 1;
      info->clone_total = clones.size();
      return;
    }
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
  auto n = number.get();
  if (n < 1)
    return;

  if (clones.size() > n)
  {
    clones.resize(n);
  }

  while (clones.size() < n)
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

  // Setup clones
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
  if (path.reached(path_index))
    if (!visitor.visit(*this, path, path_index))
      return;
}

void Clone::accept(WriteVisitor& visitor,
                   const Path& path, unsigned path_index)
{
  if (path.reached(path_index))
    if (!visitor.visit(*this, path, path_index))
      return;
  auto cv = visitor.get_sub_clone_visitor(*this);
  if (!cv)
    return;
  for (auto& graph: clones)
    graph.graph->accept(*cv, path, path_index);
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
