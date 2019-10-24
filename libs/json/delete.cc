//==========================================================================
// ViGraph JSON: delete.cc
//
// JSON delete functions
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-json.h"

namespace ViGraph { namespace JSON {

void DeleteVisitor::visit(Dataflow::Engine&,
                          const Dataflow::Path&, unsigned)
{
}

unique_ptr<Dataflow::WriteVisitor> DeleteVisitor::get_root_graph_visitor(
                          const Dataflow::Path&, unsigned)
{
  return make_unique<DeleteVisitor>(engine, "root", &engine.get_graph());
}

bool DeleteVisitor::visit(Dataflow::Graph& graph,
                          const Dataflow::Path& path, unsigned path_index)
{
  if (!path.reached(path_index))
    return true;
  graph.shutdown();
  if (scope_graph)
    scope_graph->remove(id);
  return false;
}

bool DeleteVisitor::visit(Dataflow::Clone& clone,
                          const Dataflow::Path& path, unsigned path_index)
{
  if (!path.reached(path_index))
    return true;
  clone.shutdown();
  if (scope_graph)
    scope_graph->remove(id);
  return false;
}

unique_ptr<Dataflow::WriteVisitor>
    DeleteVisitor::get_sub_element_visitor(Dataflow::Graph& graph,
                                           const string& id,
                                           const Dataflow::Path&,
                                           unsigned)
{
  return make_unique<DeleteVisitor>(engine, id, &graph);
}

unique_ptr<Dataflow::WriteVisitor>
    DeleteVisitor::get_sub_clone_visitor(Dataflow::Clone &,
                                         const string&,
                                         const Dataflow::Path&,
                                         unsigned)
{
  return make_unique<DeleteVisitor>(engine, id, nullptr);
}

bool DeleteVisitor::visit(Dataflow::Element& element,
                          const Dataflow::Path& path, unsigned path_index)
{
  if (!path.reached(path_index))
    return true;
  element.shutdown();
  if (scope_graph)
    scope_graph->remove(id);
  return false;
}

unique_ptr<Dataflow::WriteVisitor>
    DeleteVisitor::get_element_setting_visitor(Dataflow::GraphElement&,
                                               const string& id,
                                               const Dataflow::Path&,
                                               unsigned)
{
  return make_unique<DeleteVisitor>(engine, id, scope_graph);
}

void DeleteVisitor::visit(Dataflow::GraphElement&,
                          const Dataflow::SettingMember&,
                          const Dataflow::Path& path, unsigned path_index)
{
  if (!path.reached(path_index))
    return;
  throw(runtime_error{"Cannot delete a setting"});
}

unique_ptr<Dataflow::WriteVisitor>
    DeleteVisitor::get_element_input_visitor(Dataflow::GraphElement&,
                                             const string& id,
                                             const Dataflow::Path&,
                                             unsigned)
{
  return make_unique<DeleteVisitor>(engine, id, scope_graph);
}

void DeleteVisitor::visit(Dataflow::GraphElement& element,
                          const Dataflow::InputMember&,
                          const Dataflow::Path& path, unsigned path_index)
{
  if (!path.reached(path_index))
    return;
  auto g = dynamic_cast<Dataflow::Graph *>(&element);
  if (!g)
    throw(runtime_error{"Cannot delete an input on an element"});
  g->remove_input_pin(id);
}

unique_ptr<Dataflow::WriteVisitor>
    DeleteVisitor::get_element_output_visitor(Dataflow::GraphElement&,
                                              const string& id,
                                              const Dataflow::Path&,
                                              unsigned)
{
  return make_unique<DeleteVisitor>(engine, id, scope_graph);
}

void DeleteVisitor::visit(Dataflow::GraphElement& element,
                          const Dataflow::OutputMember&,
                          const Dataflow::Path& path, unsigned path_index)
{
  if (!path.reached(path_index))
    return;
  auto g = dynamic_cast<Dataflow::Graph *>(&element);
  if (!g)
    throw(runtime_error{"Cannot delete an output on an element"});
  g->remove_output_pin(id);
}

void DeleteVisitor::visit_graph_input_or_output(Dataflow::Graph&,
                                                const string&,
                                                const Dataflow::Path&,
                                                unsigned)
{
}

}} // namespaces
