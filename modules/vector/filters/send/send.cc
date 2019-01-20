//==========================================================================
// ViGraph dataflow module: vector/filters/send/send.cc
//
// Send to router filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module.h"
#include "../../vector-services.h"

namespace {

//==========================================================================
// Send filter
class SendFilter: public FrameFilter
{
  string tag;
  bool copy;
  shared_ptr<Router> router;

  // Filter/Element virtuals
  void configure(const File::Directory& base_dir,
                 const XML::Element& config) override;
  void accept(FramePtr frame) override;

public:
  // Construct
  SendFilter(const Dataflow::Module *module,
             const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
//  <send to="tag"/>
SendFilter::SendFilter(const Dataflow::Module *module,
                       const XML::Element& config):
  Element(module, config), FrameFilter(module, config)
{
  tag = config["to"];
  copy = config.get_attr_bool("copy");
}

//--------------------------------------------------------------------------
// Configure from XML (once we have the engine)
void SendFilter::configure(const File::Directory&,
                           const XML::Element&)
{
  auto& engine = graph->get_engine();
  router = engine.get_service<Router>("router");
}

//--------------------------------------------------------------------------
// Process some data
void SendFilter::accept(FramePtr frame)
{
  // Pass frame to router
  if (router && !tag.empty())
    router->send(tag, frame);

  // Pass it on
  if (copy)
    Generator::send(frame);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "send",
  "Send",
  "Send to router",
  "vector",
  {
    { "tag", { "Group name", Value::Type::number } },
    { "copy", { "Whether to send a copy in normal flow",
          Value::Type::boolean } }
  },
  { "VectorFrame" }, // inputs
  { "VectorFrame" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(SendFilter, module)
