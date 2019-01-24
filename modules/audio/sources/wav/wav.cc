//==========================================================================
// ViGraph dataflow module: audio/sources/wav/wav.cc
//
// Wav file source
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../audio-module.h"
#include "vg-geometry.h"
#include <cmath>
#include <SDL_audio.h>

namespace {

using namespace ViGraph::Geometry;

//==========================================================================
// Wav source
class WavSource: public Source
{
  File::Path file;
  bool loop = false;
  byte *buffer = nullptr;
  uint64_t samples = 0;
  unsigned int channels = 2;

  // Source/Element virtuals
  void configure(const File::Directory& base_dir,
                 const XML::Element& config) override;
  void set_property(const string& property, const SetParams& sp) override;
  void tick(const TickData& td) override;

public:
  WavSource(const Dataflow::Module *module, const XML::Element& config):
    Element(module, config), Source(module, config) {}
  ~WavSource();
};

//--------------------------------------------------------------------------
// Construct from XML:
//   <file> attributes:
//    path: path to sound file
void WavSource::configure(const File::Directory&,
                          const XML::Element& config)
{
  file = File::Path{config.get_attr("file")};
  loop = config.get_attr_bool("loop");
  if (!file.exists())
  {
    Log::Error log;
    log << "File not found: '" << file << "' in Wav '" << id << "'\n";
    return;
  }

  SDL_AudioSpec spec;
  Uint32 length = 0;
  if (!SDL_LoadWAV(file.c_str(), &spec, &buffer, &length))
  {
    Log::Error log;
    log << "File cannot be loaded: '" << file << "': " << SDL_GetError()
        << " in Wav '" << id << "'\n";
    return;
  }

  channels = spec.channels;
  SDL_AudioCVT cvt;
  if (SDL_BuildAudioCVT(&cvt, spec.format, channels, spec.freq,
                              AUDIO_F32, channels, sample_rate) < 0)
  {
    Log::Error log;
    log << "Cannot prepare file for format conversion: '" << file << "': "
        << SDL_GetError() << " in Wav '" << id << "'\n";
    SDL_FreeWAV(buffer);
    buffer = nullptr;
    return;
  }

  if (cvt.needed)
  {
    const auto len_needed = length * cvt.len_mult;
    cvt.len = length;
    if (len_needed > length)
    {
      // We're going to need a bigger boat
      // !!! - is a malloc like this safe to free with FreeWAV later?
      const auto b = static_cast<Uint8 *>(SDL_malloc(len_needed));
      memcpy(b, buffer, length);
      SDL_FreeWAV(buffer);
      buffer = b;
      length = len_needed;
    }
    cvt.buf = buffer;
    if (SDL_ConvertAudio(&cvt))
    {
      Log::Error log;
      log << "Could not convert file to usable format: '" << file << "': "
          << SDL_GetError() << " in Wav '" << id << "'\n";
      SDL_FreeWAV(buffer);
      buffer = nullptr;
      return;
    }
    buffer = cvt.buf;
    samples = cvt.len_ratio * cvt.len / sizeof(sample_t);
  }
  else
  {
    samples = length / sizeof(sample_t);
  }

  Log::Detail dlog;
  dlog << "File loaded: " << file << "' in Wav '" << id << "'\n";
}

//--------------------------------------------------------------------------
// Set a control property
void WavSource::set_property(const string& property, const SetParams& sp)
{
  if (property == "loop")
    update_prop(loop, sp);
}

//--------------------------------------------------------------------------
// Generate a fragment
void WavSource::tick(const TickData& td)
{
  const auto nsamples = td.samples(sample_rate) * channels;
  auto pos = td.sample_pos(sample_rate) * channels;
  auto fragment = new Fragment(td.t, channels);
  fragment->waveform.reserve(nsamples);
  for (auto i=0u; i<nsamples; ++i, ++pos)
  {
    if (loop && samples && pos >= samples)
      pos %= samples;
    if (pos < samples)
      fragment->waveform.push_back(
          *reinterpret_cast<sample_t *>(&buffer[pos * sizeof(sample_t)]));
    else
      fragment->waveform.push_back(0.0);
  }

  // Send to output
  send(fragment);
}

//--------------------------------------------------------------------------
// Destructor
WavSource::~WavSource()
{
  if (buffer)
  {
    SDL_FreeWAV(buffer);
    buffer = nullptr;
  }
}

Dataflow::Module module
{
  "wav",
  "Wav",
  "Audio Wav Input",
  "audio",
  {
    { "file",  { "File path", Value::Type::file, "@file" } },
    { "loop",  { "Loop", Value::Type::boolean, "@loop", true } }
  },
  {},  // no inputs
  { "Audio" }  // outputs
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(WavSource, module)
