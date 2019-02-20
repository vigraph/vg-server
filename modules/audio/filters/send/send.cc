//==========================================================================
// ViGraph dataflow module: audio/filters/send/send.cc
//
// Send to router filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../audio-module.h"
#include "../../../module-services.h"

namespace {

using namespace ViGraph::Module;

//==========================================================================
// Send filter
class SendFilter: public FragmentFilter
{
  string tag;
  bool copy;
  shared_ptr<Router> router;

  // Filter/Element virtuals
  void configure(const File::Directory& base_dir,
                 const XML::Element& config) override;
  void accept(FragmentPtr fragment) override;

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
  Element(module, config), FragmentFilter(module, config)
{
  tag = config["to"];
  if (!tag.empty()) tag = "audio:"+tag;
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
void SendFilter::accept(FragmentPtr fragment)
{
  // Pass frame to router
  if (router && !tag.empty())
    router->send(tag, fragment);

  // Pass it on
  if (copy)
    Generator::send(fragment);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "audio:send",  // ! until we have namespacing !
  "Audio Send",
  "Send audio to router",
  "audio",
  {
    { "to", { "Router tag to send to", Value::Type::text, "@to" } },
    { "copy", { "Whether to send a copy in normal flow",
          Value::Type::boolean, "@copy" } }
  },
  { "Audio" }, // inputs
  { "Audio" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(SendFilter, module)