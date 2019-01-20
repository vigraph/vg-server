//==========================================================================
// ViGraph dataflow module: vector/sources/receive/receive.cc
//
// Receive from router source
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module.h"
#include "../../vector-services.h"

namespace {

//==========================================================================
// Figure source
class ReceiveSource: public Dataflow::Source, public Router::Receiver
{
  string tag;
  shared_ptr<Router> router;

  // Source/Element virtuals
  void configure(const File::Directory& base_dir,
                 const XML::Element& config) override;
  void enable() override;
  void disable() override;

  // Receiver implementation
  void receive(FramePtr frame) override;

public:
  ReceiveSource(const Dataflow::Module *module, const XML::Element& config):
    Element(module, config), Source(module, config) {}
};

//--------------------------------------------------------------------------
// Construct from XML:
//    <receive from="tag"/>
void ReceiveSource::configure(const File::Directory&,
                              const XML::Element& config)
{
  tag = config["from"];
  auto& engine = graph->get_engine();
  router = engine.get_service<Router>("router");
}

//--------------------------------------------------------------------------
// Enable - register on router
void ReceiveSource::enable()
{
  if (router && !tag.empty())
    router->register_receiver(tag, this);
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
void ReceiveSource::receive(FramePtr frame)
{
  // Copy the frame so we can modify it without affecting others
  FramePtr copy_frame(new Frame(*frame));
  send(copy_frame);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "receive",
  "Receive",
  "Receive from router",
  "vector",
  {
    { "tag", { "Group name", Value::Type::number } }
  },
  {}, // no inputs
  { "VectorFrame" }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ReceiveSource, module)

