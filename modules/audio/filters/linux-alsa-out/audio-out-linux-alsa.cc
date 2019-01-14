//==========================================================================
// ViGraph dataflow module:
//    audio/filters/linux-alsa-out/audio-out-linux-alsa.cc
//
// Filter to output audio to the Linux ALSA output
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../audio-module.h"
#include <alsa/asoundlib.h>

namespace {

using namespace ViGraph::Dataflow;

//==========================================================================
// LinuxAudioOut filter
class LinuxALSAOutFilter: public FrameFilter
{
  // Source/Element virtuals
  void configure(const File::Directory& base_dir,
                 const XML::Element& config) override;
  void accept(FramePtr frame) override;
  void shutdown() override;

public:
  LinuxALSAOutFilter(const Dataflow::Module *module,
                     const XML::Element& config):
    Element(module, config), FrameFilter(module, config) {}
};

//--------------------------------------------------------------------------
// Configure from XML
void LinuxALSAOutFilter::configure(const File::Directory&,
                                   const XML::Element& /*config*/)
{
  Log::Streams log;
  log.detail << "Created Linux ALSA audio out\n";
}

//--------------------------------------------------------------------------
// Process some data
void LinuxALSAOutFilter::accept(FramePtr frame)
{
  // !!! Send out the frame

  // Send it down as well, so these can be chained
  send(frame);
}

//--------------------------------------------------------------------------
// Shut down
void LinuxALSAOutFilter::shutdown()
{
  Log::Detail log;
  log << "Shutting down Linux ALSA audio out\n";
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "audio-out",
  "Audio output",
  "Audio output for Linux/ALSA",
  "audio",
  { }, // !!! properties
  { "AudioFrame" }, // inputs
  { "AudioFrame" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(LinuxALSAOutFilter, module)

