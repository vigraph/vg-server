//==========================================================================
// ViGraph dataflow module: vector/sources/receive/receive.cc
//
// Receive from router source
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module.h"
#include "../../../module-services.h"

namespace {

using namespace ViGraph::Module;

//==========================================================================
// Figure source
class ReceiveSource: public Dataflow::Source, public Router::Receiver
{
public:
  string tag;

private:
  shared_ptr<Router> router;

  // Source/Element virtuals
  void setup() override;
  void enable() override;
  void disable() override;

  // Receiver implementation
  void receive(DataPtr data) override;

public:
  ReceiveSource(const Dataflow::Module *module, const XML::Element& config):
    Source(module, config) {}
};

//--------------------------------------------------------------------------
// Setup
void ReceiveSource::setup()
{
  auto& engine = graph->get_engine();
  router = engine.get_service<Router>("router");
}

//--------------------------------------------------------------------------
// Enable - register on router
void ReceiveSource::enable()
{
  if (router && !tag.empty())
    router->register_receiver("vector:" + tag, this);
}

//--------------------------------------------------------------------------
// Disable - deregister from router
void ReceiveSource::disable()
{
  if (router)
    router->deregister_receiver(this);
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

