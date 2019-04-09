//==========================================================================
// ViGraph dataflow module: audio/sources/receive/receive.cc
//
// Receive from router source
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../audio-module.h"

namespace {

using namespace ViGraph::Module;

//==========================================================================
// Figure source
class ReceiveSource: public Dataflow::Source, public Router::Receiver
{
  // Source/Element virtuals
  void calculate_topology(Element::Topology& topo) override;
  void enable() override;
  void disable() override;

  // Receiver implementation
  void receive(DataPtr data) override;

public:
  string tag;

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
    router.register_receiver("audio:" + tag, this);
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
  FragmentPtr copy_frag(new Fragment(*data.check<Fragment>()));
  send(copy_frag);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "audio:receive", // ! until we have namespacing !
  "Audio Receive",
  "Receive audio from router",
  "audio",
  {
    { "from", { "Router tag to receive from", Value::Type::text,
                &ReceiveSource::tag, false } }
  },
  {}, // no inputs
  { "Audio" }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ReceiveSource, module)

