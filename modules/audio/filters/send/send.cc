//==========================================================================
// ViGraph dataflow module: audio/filters/send/send.cc
//
// Send to router filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../audio-module.h"

namespace {

using namespace ViGraph::Module;

//==========================================================================
// Send filter
class SendFilter: public FragmentFilter
{
  // Filter/Element virtuals
  void accept(FragmentPtr fragment) override;

public:
  string tag;
  bool copy = false;

  using FragmentFilter::FragmentFilter;
};

//--------------------------------------------------------------------------
// Process some data
void SendFilter::accept(FragmentPtr fragment)
{
  auto& router = graph->get_engine().router;

  // Pass frame to router
  if (!tag.empty())
    router.send("audio:" + tag, fragment);

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
    { "to", { "Router tag to send to", Value::Type::text,
              &SendFilter::tag, false } },
    { "copy", { "Whether to send a copy in normal flow", Value::Type::boolean,
                &SendFilter::copy, true } }
  },
  { "Audio" }, // inputs
  { "Audio" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(SendFilter, module)
