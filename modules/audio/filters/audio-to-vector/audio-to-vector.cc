//==========================================================================
// ViGraph dataflow module:
//    audio/filters/audio-to-vector/audio-to-vector.cc
//
// Audio audio-to-vector filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../audio-module.h"
#include "../../../vector/vector-module.h"
#include "../../../module-services.h"

namespace {

using namespace ViGraph::Dataflow;
using namespace ViGraph::Module;

//==========================================================================
// VU meter filter
class AudioToVectorFilter: public FragmentFilter
{
private:
  string tag;
  FramePtr frame{new Frame{0}};
  shared_ptr<Router> router;

  // Filter/Element virtuals
  void configure(const File::Directory& base_dir,
                 const XML::Element& config) override;
  void accept(FragmentPtr fragment) override;

public:
  AudioToVectorFilter(const Dataflow::Module *module,
                      const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML:
//   <audio-to-vector/>
AudioToVectorFilter::AudioToVectorFilter(const Dataflow::Module *module,
                                         const XML::Element& config):
  FragmentFilter(module, config)
{
  tag = config["to"];
  if (!tag.empty())
    tag = "vector:" + tag;
}

//--------------------------------------------------------------------------
// Configure from XML (once we have the engine)
void AudioToVectorFilter::configure(const File::Directory&,
                                    const XML::Element&)
{
  auto& engine = graph->get_engine();
  router = engine.get_service<Router>("router");
}

//--------------------------------------------------------------------------
// Process some data
void AudioToVectorFilter::accept(FragmentPtr fragment)
{
  for (const auto& wit: fragment->waveforms)
  {
    for (const auto& s: wit.second)
    {
      frame->points.emplace_back(static_cast<double>(frame->points.size())
                                 / wit.second.size() - 0.5,
                                 max(-1.0f, min(1.0f, s)) / 2,
                                 frame->points.empty() ? Colour::black
                                                       : Colour::white);
    }
    if (router && !tag.empty())
      router->send(tag, frame);
    frame->points.clear();
    // Single channel only for now
    break;
  }
  send(fragment);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "audio-to-vector",
  "Audio-to-Vector",
  "Send copy of audio data to vector data via router",
  "audio",
  {
    { "to", { "Router tag to send to", Value::Type::text, "@to" } },
  },
  { "Audio" }, // inputs
  { "Audio" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(AudioToVectorFilter, module)
