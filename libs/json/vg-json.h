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
  unique_ptr<ReadVisitor> get_root_graph_visitor() override;
  void visit(const Dataflow::Graph& graph,
             const Dataflow::Path& path, unsigned path_index) override;
  void visit(const Dataflow::Clone& graph,
             const Dataflow::Path& path, unsigned path_index) override;
  unique_ptr<ReadVisitor> get_sub_element_visitor(const string& id) override;
  void visit(const Dataflow::Element& element,
             const Dataflow::Path& path, unsigned path_index) override;
};

//==========================================================================
// Set Visitor
class SetVisitor: public Dataflow::WriteVisitor
{
private:
  const Dataflow::Engine& engine;
  const Value& json;
  Dataflow::Graph *scope_graph = nullptr;
  Dataflow::Clone *clone = nullptr;
  map<string, const Value *> sub_element_json;

public:
  SetVisitor(const Dataflow::Engine& _engine, const Value& _json,
             Dataflow::Graph *_scope_graph = nullptr,
             Dataflow::Clone *_clone = nullptr):
    engine{_engine}, json{_json}, scope_graph{_scope_graph}, clone{_clone}
  {}
  void visit(Dataflow::Engine& engine,
             const Dataflow::Path& path, unsigned path_index) override;
  unique_ptr<WriteVisitor> get_root_graph_visitor() override;
  void visit(Dataflow::Graph& graph,
             const Dataflow::Path& path, unsigned path_index) override;
  void visit(Dataflow::Clone& graph,
             const Dataflow::Path& path, unsigned path_index) override;
  unique_ptr<WriteVisitor> get_sub_element_visitor(const string& id,
                                            Dataflow::Graph& scope) override;
  unique_ptr<WriteVisitor> get_sub_clone_visitor(
                                            Dataflow::Clone& clone) override;
  void visit(Dataflow::Element& element,
             const Dataflow::Path& path, unsigned path_index) override;
};

//==========================================================================
}} //namespaces
#endif // !__VG_JSON_H
