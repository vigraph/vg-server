//==========================================================================
// ViGraph JSON: delete.cc
//
// JSON delete functions
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-json.h"

namespace ViGraph { namespace JSON {

bool DeleteVisitor::visit(Dataflow::Engine&,
                          const Dataflow::Path&, unsigned)
{
  return true;
}

unique_ptr<Dataflow::WriteVisitor> DeleteVisitor::get_root_graph_visitor()
{
  return make_unique<DeleteVisitor>(engine, "root", &engine.get_graph());
}

bool DeleteVisitor::visit(Dataflow::Graph& graph,
                          const Dataflow::Path&, unsigned)
{
  graph.shutdown();
  if (scope_graph)
    scope_graph->remove(id);
  return false;
}

bool DeleteVisitor::visit(Dataflow::Clone& clone,
                          const Dataflow::Path&, unsigned)
{
  clone.shutdown();
  if (scope_graph)
    scope_graph->remove(id);
  return false;
}

unique_ptr<Dataflow::WriteVisitor>
    DeleteVisitor::get_sub_element_visitor(const string& id, bool,
                                           Dataflow::Graph &scope)
{
  return make_unique<DeleteVisitor>(engine, id, &scope);
}

unique_ptr<Dataflow::WriteVisitor>
    DeleteVisitor::get_sub_clone_visitor(Dataflow::Clone &)
{
  return make_unique<DeleteVisitor>(engine, id, nullptr);
}

bool DeleteVisitor::visit(Dataflow::Element& element,
                          const Dataflow::Path&, unsigned)
{
  element.shutdown();
  if (scope_graph)
    scope_graph->remove(id);
  return false;
}

unique_ptr<Dataflow::WriteVisitor>
    DeleteVisitor::get_element_setting_visitor(const string& id, bool)
{
  return make_unique<DeleteVisitor>(engine, id, scope_graph);
}

bool DeleteVisitor::visit(Dataflow::GraphElement&,
                          const Dataflow::SettingMember&,
                          const Dataflow::Path&, unsigned)
{
  throw(runtime_error{"Cannot delete a setting"});
}

unique_ptr<Dataflow::WriteVisitor>
    DeleteVisitor::get_element_input_visitor(const string& id, bool)
{
  return make_unique<DeleteVisitor>(engine, id, scope_graph);
}

bool DeleteVisitor::visit(Dataflow::GraphElement& element,
                          const Dataflow::InputMember&,
                          const Dataflow::Path&, unsigned)
{
  auto g = dynamic_cast<Dataflow::Graph *>(&element);
  if (!g)
    throw(runtime_error{"Cannot delete an input on an element"});
  g->remove_input_pin(id);
  return true;
}

unique_ptr<Dataflow::WriteVisitor>
    DeleteVisitor::get_element_output_visitor(const string& id, bool)
{
  return make_unique<DeleteVisitor>(engine, id, scope_graph);
}

bool DeleteVisitor::visit(Dataflow::GraphElement& element,
                          const Dataflow::OutputMember&,
                          const Dataflow::Path&, unsigned)
{
  auto g = dynamic_cast<Dataflow::Graph *>(&element);
  if (!g)
    throw(runtime_error{"Cannot delete an output on an element"});
  g->remove_output_pin(id);
  return true;
}

bool DeleteVisitor::visit_graph_input_or_output(Dataflow::Graph&,
                                                const string&,
                                                bool)
{
  return true;
}

}} // namespaces
