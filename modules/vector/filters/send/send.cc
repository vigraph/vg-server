//==========================================================================
// ViGraph dataflow module: vector/filters/send/send.cc
//
// Send to router filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module.h"
#include "../../../module-services.h"

namespace {

using namespace ViGraph::Module;

//==========================================================================
// Send filter
class SendFilter: public FrameFilter
{
public:
  string tag;
  bool copy = false;

private:
  shared_ptr<Router> router;

  // Filter/Element virtuals
  void setup() override;
  void accept(FramePtr frame) override;

public:
  using FrameFilter::FrameFilter;
};

//--------------------------------------------------------------------------
// Setup
void SendFilter::setup()
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
    router->send("vector:" + tag, frame);

  // Pass it on
  if (copy)
    Generator::send(frame);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "send",
  "Vector Send",
  "Send vector data to router",
  "vector",
  {
    { "to", { "Router tag to send to", Value::Type::text,
              &SendFilter::tag, false } },
    { "copy", { "Whether to send a copy in normal flow", Value::Type::boolean,
                &SendFilter::copy, true } }
  },
  { "VectorFrame" }, // inputs
  { "VectorFrame" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(SendFilter, module)
