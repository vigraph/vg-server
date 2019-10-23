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
Clone *Clone::clone() const
{
  auto c = new Clone{clone_module};
  for (const auto& graph: clones)
  {
    auto g = graph.graph->clone();
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
    auto g = clones.front().graph->clone();
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
  {
    visitor.visit(*this, path, path_index);

    if (module.has_settings())
    {
      module.for_each_setting([this, &visitor, &path, &path_index]
                            (const string& id,
                             const SettingMember& setting)
      {
        auto iv = visitor.get_element_setting_visitor(*this, id,
                                                      path, path_index);
        if (iv)
          setting.accept(*iv, path, path_index + 1, *this);
      });
    }
  }
}

void Clone::accept(WriteVisitor& visitor,
                   const Path& path, unsigned path_index)
{
  if (path.reached(path_index))
  {
    visitor.visit(*this, path, path_index);

    if (module.has_settings())
    {
      module.for_each_setting([this, &visitor, &path, &path_index]
                            (const string& id,
                             const SettingMember& setting)
      {
        auto iv = visitor.get_element_setting_visitor(*this, id,
                                                      path, path_index);
        if (iv)
          setting.accept(*iv, path, path_index + 1, *this);
      });
    }
    setup();

    auto cv = visitor.get_sub_clone_visitor(*this, get_id(), path, path_index);
    if (cv)
      for (auto& graph: clones)
        graph.graph->accept(*cv, path, path_index);
    return;
  }

  auto part = path.get(path_index);

  switch (part.type)
  {
    case Path::PartType::attribute:
      {
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
              throw(runtime_error{"No such attribute: " + part.name});
            }
          }
        }
      }
      break;
    case Path::PartType::element:
      {
        auto cv = visitor.get_sub_clone_visitor(*this, part.name,
                                                path, path_index);
        if (!cv)
          return;
        for (auto& graph: clones)
          graph.graph->accept(*cv, path, path_index);
      }
      break;
    default:
      break;
  }
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
