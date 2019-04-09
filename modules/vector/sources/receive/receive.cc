//==========================================================================
// ViGraph dataflow module: vector/sources/receive/receive.cc
//
// Receive from router source
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module.h"

namespace {

using namespace ViGraph::Module;

//==========================================================================
// Figure source
class ReceiveSource: public Dataflow::Source, public Router::Receiver
{
public:
  string tag;

private:
  // Source/Element virtuals
  void calculate_topology(Element::Topology& topo) override;
  void enable() override;
  void disable() override;

  // Receiver implementation
  void receive(DataPtr data) override;

public:
  using Source::Source;
};

//--------------------------------------------------------------------------
// Topology calculation - register as receiver
void ReceiveSource::calculate_topology(Element::Topology& topo)
{
  topo.router_receivers[tag].push_back(this);
}

//--------------------------------------------------------------------------
// Enable - register on router
void ReceiveSource::enable()
{
  auto& router = graph->get_engine().router;

  if (!tag.empty())
    router.register_receiver("vector:" + tag, this);
}

//--------------------------------------------------------------------------
// Disable - deregister from router
void ReceiveSource::disable()
{
  auto& router = graph->get_engine().router;
  router.deregister_receiver(this);
}

//--------------------------------------------------------------------------
// Receiver implementation
void ReceiveSource::receive(DataPtr data)
{
  // Copy the frame so we can modify it without affecting others
  FramePtr copy_frame(new Frame(*data.check<Frame>()));
  send(copy_frame);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "receive",
  "Vector Receive",
  "Receive vector data from router",
  "vector",
  {
    { "from", { "Router tag to receive from", Value::Type::text,
                &ReceiveSource::tag, false } }
  },
  {}, // no inputs
  { "VectorFrame" }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ReceiveSource, module)

