//==========================================================================
// ViGraph JSON conversion: vg-json.h
//
// JSON <-> dataflow functions
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VG_JSON_H
#define __VG_JSON_H

#include "ot-json.h"
#include "vg-dataflow.h"

namespace ViGraph { namespace JSON {

// Make our lives easier without polluting anyone else
using namespace std;
using namespace ObTools;
using namespace ViGraph;
using ObTools::JSON::Value;

//==========================================================================
// Get metadata for module
Value get_module_metadata(const Dataflow::Module& module);

//==========================================================================
// Get Visitor
class GetVisitor: public Dataflow::ReadVisitor
{
private:
  Value& json;
  set<string> output_pins;
  bool no_connections = false;

public:
  GetVisitor(Value& _json, bool _no_connections = false):
    json{_json}, no_connections{_no_connections}
  {}
  void visit(const Dataflow::Engine& engine,
             const Dataflow::Path& path, unsigned path_index) override;
  unique_ptr<ReadVisitor> get_root_graph_visitor(
             const Dataflow::Path& path, unsigned path_index) override;
  void visit(const Dataflow::Graph& graph,
             const Dataflow::Path& path, unsigned path_index) override;
  void visit(const Dataflow::Clone& graph,
             const Dataflow::Path& path, unsigned path_index) override;
  unique_ptr<ReadVisitor> get_sub_element_visitor(
                                      const Dataflow::Graph& graph,
                                      const string& id,
                                      const Dataflow::Path& path,
                                      unsigned path_index) override;
  void visit(const Dataflow::Element& element,
             const Dataflow::Path& path, unsigned path_index) override;
  unique_ptr<ReadVisitor> get_element_setting_visitor(
                                      const Dataflow::GraphElement& element,
                                      const string& id,
                                      const Dataflow::Path& path,
                                      unsigned path_index) override;
  void visit(const Dataflow::GraphElement& element,
             const Dataflow::SettingMember& setting,
             const Dataflow::Path& path, unsigned path_index) override;
  unique_ptr<ReadVisitor> get_element_input_visitor(
                                      const Dataflow::GraphElement& element,
                                      const string& id,
                                      const Dataflow::Path& path,
                                      unsigned path_index) override;
  void visit(const Dataflow::GraphElement& element,
             const Dataflow::InputMember& input,
             const Dataflow::Path& path, unsigned path_index) override;
  unique_ptr<ReadVisitor> get_element_output_visitor(
                                      const Dataflow::GraphElement& element,
                                      const string& id,
                                      const Dataflow::Path& path,
                                      unsigned path_index) override;
  void visit(const Dataflow::GraphElement& element,
             const Dataflow::OutputMember& output,
             const Dataflow::Path& path, unsigned path_index) override;
  void visit_graph_input_or_output(const Dataflow::Graph& graph,
                                   const string& id,
                                   const Dataflow::Path& path,
                                   unsigned path_index) override;
};

//==========================================================================
// Set Visitor
class SetVisitor: public Dataflow::WriteVisitor
{
private:
  const Dataflow::Engine& engine;
  const Value& json;
  const string id;
  Dataflow::Graph *scope_graph = nullptr;
  Dataflow::Clone *clone = nullptr;
  map<string, const Value *> sub_element_json;

public:
  SetVisitor(const Dataflow::Engine& _engine, const Value& _json,
             const string& _id = "",
             Dataflow::Graph *_scope_graph = nullptr,
             Dataflow::Clone *_clone = nullptr):
    engine{_engine}, json{_json}, id{_id},
    scope_graph{_scope_graph}, clone{_clone}
  {}
  void visit(Dataflow::Engine& engine,
             const Dataflow::Path& path, unsigned path_index) override;
  unique_ptr<WriteVisitor> get_root_graph_visitor(
             const Dataflow::Path& path, unsigned path_index) override;
  bool visit(Dataflow::Graph& graph,
             const Dataflow::Path& path, unsigned path_index) override;
  bool visit(Dataflow::Clone& clone,
             const Dataflow::Path& path, unsigned path_index) override;
  unique_ptr<WriteVisitor> get_sub_element_visitor(
                                            Dataflow::Graph& scope,
                                            const string& id,
                                            const Dataflow::Path& path,
                                            unsigned path_index) override;
  unique_ptr<WriteVisitor> get_sub_clone_visitor(
                                            Dataflow::Clone& clone,
                                            const string& id,
                                            const Dataflow::Path& path,
                                            unsigned path_index) override;
  bool visit(Dataflow::Element& element,
             const Dataflow::Path& path, unsigned path_index) override;
  unique_ptr<WriteVisitor> get_element_setting_visitor(
                                              Dataflow::GraphElement& element,
                                              const string& id,
                                              const Dataflow::Path& path,
                                              unsigned path_index) override;
  void visit(Dataflow::GraphElement& element,
             const Dataflow::SettingMember& setting,
             const Dataflow::Path& path, unsigned path_index) override;
  unique_ptr<WriteVisitor> get_element_input_visitor(
                                              Dataflow::GraphElement& element,
                                              const string& id,
                                              const Dataflow::Path& path,
                                              unsigned path_index) override;
  void visit(Dataflow::GraphElement& element,
             const Dataflow::InputMember& input,
             const Dataflow::Path& path, unsigned path_index) override;
  unique_ptr<WriteVisitor> get_element_output_visitor(
                                              Dataflow::GraphElement& element,
                                              const string& id,
                                              const Dataflow::Path& path,
                                              unsigned path_index) override;
  void visit(Dataflow::GraphElement& element,
             const Dataflow::OutputMember& output,
             const Dataflow::Path& path, unsigned path_index) override;
  void visit_graph_input_or_output(Dataflow::Graph& graph,
                                   const string& id,
                                   const Dataflow::Path& path,
                                   unsigned path_index) override;
};

//==========================================================================
// Delete Visitor
class DeleteVisitor: public Dataflow::WriteVisitor
{
private:
  const Dataflow::Engine& engine;
  const string id;
  Dataflow::Graph *scope_graph = nullptr;

public:
  DeleteVisitor(const Dataflow::Engine& _engine,
                const string& _id = "",
                Dataflow::Graph *_scope_graph = nullptr):
    engine{_engine}, id{_id}, scope_graph{_scope_graph}
  {}
  void visit(Dataflow::Engine& engine,
             const Dataflow::Path& path, unsigned path_index) override;
  unique_ptr<WriteVisitor> get_root_graph_visitor(
             const Dataflow::Path& path, unsigned path_index) override;
  bool visit(Dataflow::Graph& graph,
             const Dataflow::Path& path, unsigned path_index) override;
  bool visit(Dataflow::Clone& clone,
             const Dataflow::Path& path, unsigned path_index) override;
  unique_ptr<WriteVisitor> get_sub_element_visitor(
                                            Dataflow::Graph& scope,
                                            const string& id,
                                            const Dataflow::Path& path,
                                            unsigned path_index) override;
  unique_ptr<WriteVisitor> get_sub_clone_visitor(
                                            Dataflow::Clone& clone,
                                            const string& id,
                                            const Dataflow::Path& path,
                                            unsigned path_index) override;
  bool visit(Dataflow::Element& element,
             const Dataflow::Path& path, unsigned path_index) override;
  unique_ptr<WriteVisitor> get_element_setting_visitor(
                                              Dataflow::GraphElement& element,
                                              const string& id,
                                              const Dataflow::Path& path,
                                              unsigned path_index) override;
  void visit(Dataflow::GraphElement& element,
             const Dataflow::SettingMember& setting,
             const Dataflow::Path& path, unsigned path_index) override;
  unique_ptr<WriteVisitor> get_element_input_visitor(
                                              Dataflow::GraphElement& element,
                                              const string& id,
                                              const Dataflow::Path& path,
                                              unsigned path_index) override;
  void visit(Dataflow::GraphElement& element,
             const Dataflow::InputMember& input,
             const Dataflow::Path& path, unsigned path_index) override;
  unique_ptr<WriteVisitor> get_element_output_visitor(
                                              Dataflow::GraphElement& element,
                                              const string& id,
                                              const Dataflow::Path& path,
                                              unsigned path_index) override;
  void visit(Dataflow::GraphElement& element,
             const Dataflow::OutputMember& output,
             const Dataflow::Path& path, unsigned path_index) override;
  void visit_graph_input_or_output(Dataflow::Graph& graph,
                                   const string& id,
                                   const Dataflow::Path& path,
                                   unsigned path_index) override;
};

//==========================================================================
}} //namespaces
#endif // !__VG_JSON_H
