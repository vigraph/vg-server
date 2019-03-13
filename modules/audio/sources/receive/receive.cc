//==========================================================================
// ViGraph dataflow module: audio/sources/receive/receive.cc
//
// Receive from router source
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../audio-module.h"
#include "../../../module-services.h"

namespace {

using namespace ViGraph::Module;

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
  void receive(DataPtr data) override;

public:
  ReceiveSource(const Dataflow::Module *module, const XML::Element& config):
    Source(module, config) {}
};

//--------------------------------------------------------------------------
// Construct from XML:
//    <receive from="tag"/>
void ReceiveSource::configure(const File::Directory&,
                              const XML::Element& config)
{
  tag = config["from"];
  if (!tag.empty()) tag = "audio:"+tag;
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
    { "from", { "Router tag to receive from", Value::Type::text, "@from" } }
  },
  {}, // no inputs
  { "Audio" }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ReceiveSource, module)

