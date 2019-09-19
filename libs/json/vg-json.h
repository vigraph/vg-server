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
class GetVisitor: public Dataflow::Visitor
{
private:
  Value& json;
  set<string> output_pins;
  bool no_connections = false;

public:
  GetVisitor(Value& _json, bool _no_connections = false):
    json{_json}, no_connections{_no_connections}
  {}
  void visit(Dataflow::Engine& engine) override;
  unique_ptr<Visitor> getSubGraphVisitor() override;
  void visit(Dataflow::Graph& graph) override;
  unique_ptr<Visitor> getSubElementVisitor(const string& id) override;
  void visit(Dataflow::Element& element) override;
};

//==========================================================================
// Set Visitor
class SetVisitor: public Dataflow::Visitor
{
private:
  const Dataflow::Engine& engine;
  const Value& json;
  Dataflow::Graph *scope_graph = nullptr;
  map<string, const Value *> sub_element_json;

public:
  SetVisitor(const Dataflow::Engine& _engine, const Value& _json,
             Dataflow::Graph *_scope_graph = nullptr):
    engine{_engine}, json{_json}, scope_graph{_scope_graph}
  {}
  void visit(Dataflow::Engine& engine) override;
  unique_ptr<Visitor> getSubGraphVisitor() override;
  void visit(Dataflow::Graph& graph) override;
  unique_ptr<Visitor> getSubElementVisitor(const string& id) override;
  void visit(Dataflow::Element& element) override;
};

//==========================================================================
}} //namespaces
#endif // !__VG_JSON_H
