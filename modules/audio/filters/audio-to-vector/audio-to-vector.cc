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
  enum class Mode
  {
    multi,
    combined,
    first
  } mode = Mode::multi;

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
  auto m = config["mode"];
  if (m.empty() || m == "multi")
    mode = Mode::multi;
  else if (m == "combined")
    mode = Mode::combined;
  else if (m == "first")
    mode = Mode::first;
  else
  {
    Log::Error log;
    log << "Unknown mode '" << m << "' in " << id << endl;
  }
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
  frame->points.resize(0);
  auto c = 0u;
  auto s = 0u;
  const auto channels = fragment->waveforms.size();
  for (const auto& wit: fragment->waveforms)
  {
    const auto& w = wit.second;
    if (mode == Mode::multi)
      frame->points.resize(frame->points.size() + w.size());
    else
      frame->points.resize(max(frame->points.size(), w.size()));
    for (auto i = 0u; i < w.size(); ++i)
    {
      const auto a = max(-1.0f, min(1.0f, w[i]));
      switch (mode)
      {
        case Mode::multi:
          frame->points[s] = {static_cast<double>(i) / w.size() - 0.5,
                              ((a + 1.0) / 2) / channels
                                - static_cast<double>(c + 1) / channels,
                              i ? Colour::white : Colour::black};
          break;
        case Mode::combined:
          if (c)
            frame->points[i].y += (a / channels) / 2;
          else
            frame->points[i] = {static_cast<double>(i) / w.size() - 0.5,
                                (a / channels) / 2,
                                i ? Colour::white : Colour::black};
          break;
        case Mode::first:
          frame->points[i] = {static_cast<double>(i) / w.size() - 0.5,
                              a / 2, i ? Colour::white : Colour::black};
          break;
      }
      ++s;
    }
    if (mode == Mode::first)
      break;
    ++c;
  }
  if (!frame->points.empty())
  {
    if (router && !tag.empty())
      router->send(tag, frame);
    frame->points.clear();
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
