//==========================================================================
// ViGraph JSON: delete.cc
//
// JSON delete functions
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-json.h"

namespace ViGraph { namespace JSON {

class DeleteVisitor: public Dataflow::WriteVisitor
{
private:
  const string id;
  const Dataflow::Path::PartType type;
public:
  DeleteVisitor(const string& _id, Dataflow::Path::PartType _type):
    id{_id}, type{_type}
  {}
  void visit(Dataflow::Engine& engine) override;
  void visit(Dataflow::Graph& graph) override;
  void visit(Dataflow::Clone& clone) override;
  void visit(Dataflow::Element& element) override;
  void visit(const Dataflow::SettingMember& setting) override;
  void visit(const Dataflow::InputMember& input) override;
  void visit(const Dataflow::OutputMember& output) override;
};

void del(Dataflow::Engine& engine, const Dataflow::Path& path)
{
  auto lock = engine.get_write_lock();

  const auto parent = path.parent();
  const auto id = path.leaf();
  const auto type = path.type();
  auto acceptors = engine.get_visitor_acceptors(parent, 0);
  if (acceptors.empty())
    throw runtime_error("Path not found");

  auto visitor = DeleteVisitor{id, type};
  for (auto& a: acceptors)
  {
    if (!a.acceptor)
      throw runtime_error("Path not found");
    a.acceptor->accept(visitor);
  }

  engine.update_elements();
}


void DeleteVisitor::visit(Dataflow::Engine&)
{
  throw(runtime_error{"Cannot delete"});
}

void DeleteVisitor::visit(Dataflow::Graph& graph)
{
  switch (type)
  {
    case Dataflow::Path::PartType::element:
      graph.remove(id);
      break;
    case Dataflow::Path::PartType::attribute:
      graph.remove_input_pin(id);
      graph.remove_output_pin(id);
      break;
    default:
      break;
  }
}

void DeleteVisitor::visit(Dataflow::Clone&)
{
  throw(runtime_error{"Cannot delete"});
}

void DeleteVisitor::visit(Dataflow::Element&)
{
  throw(runtime_error{"Cannot delete"});
}

void DeleteVisitor::visit(const Dataflow::SettingMember&)
{
  throw(runtime_error{"Cannot delete"});
}

void DeleteVisitor::visit(const Dataflow::InputMember&)
{
  throw(runtime_error{"Cannot delete"});
}

void DeleteVisitor::visit(const Dataflow::OutputMember&)
{
  throw(runtime_error{"Cannot delete"});
}

}} // namespaces
