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

public:
  GetVisitor(Value& _json): json{_json} {}
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
  const Value& json;
  Dataflow::Graph *scope_graph = nullptr;

public:
  SetVisitor(const Value& _json, Dataflow::Graph *_scope_graph = nullptr):
    json{_json}, scope_graph{_scope_graph}
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
