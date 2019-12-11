//==========================================================================
// ViGraph dataflow machines: clone.cc
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
// Get connection inputs
vector<ElementInput *> Clone::get_connection_inputs(const string& name)
{
  auto inputs = vector<ElementInput *>{};
  for (auto& graph: clones)
  {
    const auto gi = graph.graph->get_connection_inputs(name);
    inputs.insert(inputs.end(), gi.begin(), gi.end());
  }
  return inputs;
}

//--------------------------------------------------------------------------
// Notify of a connection
void Clone::notify_connection(const string& /*in_name*/,
                              GraphElement& /*a*/, const string& /*out_name*/)
{
  //throw(runtime_error("Unimplemented"));
}

//--------------------------------------------------------------------------
// Update clone infos
void Clone::update_clone_infos()
{
  auto i = 0u;
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
// Clone
Clone *Clone::clone(const SetupContext& context) const
{
  auto c = new Clone{clone_module};
  for (const auto& graph: clones)
  {
    auto g = graph.graph->clone(context);
    c->clones.emplace_back(g);
    const auto& els = g->get_elements();
    for (const auto& el: els)
    {
      auto info = dynamic_cast<Dataflow::CloneInfo *>(el.second.get());
      if (info)
        c->register_info(*g, info);
    }
  }
  c->update_clone_infos();
  return c;
}

//--------------------------------------------------------------------------
// Get graphs
vector<Graph *> Clone::get_graphs() const
{
  auto graphs = vector<Graph *>{};
  for (const auto& graph: clones)
  {
    graphs.push_back(graph.graph.get());
  }
  return graphs;
}

//--------------------------------------------------------------------------
// Final setup for elements and calculate topology
void Clone::setup(const SetupContext& context)
{
  const auto n = copies.get();
  if (n < 1)
    return;

  if (clones.size() > n)
  {
    clones.resize(n);
  }

  while (clones.size() < n)
  {
    auto g = clones.front().graph->clone(context);
    clones.emplace_back(g);
    const auto& els = g->get_elements();
    for (const auto& el: els)
    {
      auto info = dynamic_cast<Dataflow::CloneInfo *>(el.second.get());
      if (info)
        register_info(*g, info);
    }
  }

  update_clone_infos();

  // Setup clones
  for (const auto& graph: clones)
    graph.graph->setup(context);
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
// Shutdown all elements
void Clone::shutdown()
{
  for (auto& graph: clones)
    graph.graph->shutdown();
}

//--------------------------------------------------------------------------
// Pathing
vector<ConstVisitorAcceptorInfo> Clone::get_visitor_acceptors(
                                                  const Path& path,
                                                  unsigned path_index,
                                                  const Graph *graph,
                                                  const Clone *) const
{
  if (path.reached(path_index))
    return {{get_id(), this, graph, this}};

  if (path.type(path_index) == Path::PartType::attribute)
  {
    const auto& name = path.name(path_index);

    const auto s = module.get_setting(name);
    if (s)
      return {{name, s, this, graph}};

    const auto i = module.get_input(name);
    if (i)
      return {{name, i, this, graph}};

    const auto o = module.get_output(name);
    if (o)
      return {{name, o, this, graph}};
  }

  auto graphs = vector<ConstVisitorAcceptorInfo>{};
  for (const auto& c: clones)
  {
    const auto& sub = c.graph->get_visitor_acceptors(path, path_index,
                                                     graph, this);
    for (const auto& s: sub)
      graphs.emplace_back(s);
  }
  return graphs;
}

vector<VisitorAcceptorInfo> Clone::get_visitor_acceptors(
                                                  const Path& path,
                                                  unsigned path_index,
                                                  Graph *graph,
                                                  Clone *)
{
  if (path.reached(path_index))
    return {{get_id(), static_cast<VisitorAcceptor *>(this), graph, this}};

  if (path.type(path_index) == Path::PartType::attribute)
  {
    const auto& name = path.name(path_index);

    const auto s = module.get_setting(name);
    if (s)
      return {{name, s, this, graph, true}};

    const auto i = module.get_input(name);
    if (i)
      return {{name, i, this, graph}};

    const auto o = module.get_output(name);
    if (o)
      return {{name, o, this, graph}};
  }

  auto graphs = vector<VisitorAcceptorInfo>{};
  for (const auto& c: clones)
  {
    const auto& sub = c.graph->get_visitor_acceptors(path, path_index,
                                                     graph, this);
    for (const auto& s: sub)
      graphs.emplace_back(s);
  }
  return graphs;
}

//==========================================================================
// Clone Info

//--------------------------------------------------------------------------
// Tick
void CloneInfo::tick(const TickData& td)
{
  number.get_buffer(td).data.push_back(clone_number);
  total.get_buffer(td).data.push_back(clone_total);
  const auto f = clone_total ? (clone_number - 1.0) / clone_total : 0.0;
  fraction.get_buffer(td).data.push_back(f);
}

}} // namespaces
