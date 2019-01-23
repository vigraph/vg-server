//==========================================================================
// ViGraph dataflow module: audio/sources/file/file.cc
//
// Sound file source
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
// File source
class FileSource: public Source
{
  File::Path file;
  bool loop = false;
  Uint8 *buffer = nullptr;
  Uint32 samples = 0;

  // Source/Element virtuals
  void configure(const File::Directory& base_dir,
                 const XML::Element& config) override;
  void set_property(const string& property, const SetParams& sp) override;
  void tick(const TickData& td) override;

public:
  FileSource(const Dataflow::Module *module, const XML::Element& config):
    Element(module, config), Source(module, config) {}
  ~FileSource();
};

//--------------------------------------------------------------------------
// Construct from XML:
//   <file> attributes:
//    path: path to sound file
void FileSource::configure(const File::Directory&,
                           const XML::Element& config)
{
  file = File::Path{config.get_attr("path")};
  loop = config.get_attr_bool("loop");
  if (!file.exists())
  {
    Log::Error log;
    log << "File not found: '" << file << "' in File '" << id << "'\n";
    return;
  }

  SDL_AudioSpec spec;
  Uint32 length = 0;
  if (!SDL_LoadWAV(file.c_str(), &spec, &buffer, &length))
  {
    Log::Error log;
    log << "File cannot be loaded: '" << file << "': " << SDL_GetError()
        << " in File '" << id << "'\n";
    return;
  }

  SDL_AudioCVT cvt;
  if (SDL_BuildAudioCVT(&cvt, spec.format, spec.channels, spec.freq,
                              AUDIO_F32, 1, sample_rate) < 0)
  {
    Log::Error log;
    log << "Cannot prepare file for format conversion: '" << file << "': "
        << SDL_GetError() << " in File '" << id << "'\n";
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
          << SDL_GetError() << " in File '" << id << "'\n";
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
  dlog << "File loaded: " << file << "' in File '" << id << "'\n";
}

//--------------------------------------------------------------------------
// Set a control property
void FileSource::set_property(const string& property, const SetParams& sp)
{
  if (property == "loop")
    update_prop(loop, sp);
}

//--------------------------------------------------------------------------
// Generate a fragment
void FileSource::tick(const TickData& td)
{
  const auto last_tick_total = static_cast<int>(
      floor(td.interval.seconds() * (td.global_n) * sample_rate));
  const auto tick_total = static_cast<int>(
      floor(td.interval.seconds() * (td.global_n + 1) * sample_rate));
  const auto nsamples = tick_total - last_tick_total;

  auto fragment = new Fragment(td.t);  // mono
  fragment->waveform.reserve(nsamples);
  const auto first_local_tick_total = static_cast<int>(
      floor(td.interval.seconds() * (td.global_n - td.n) * sample_rate));
  auto pos = static_cast<Uint32>(last_tick_total - first_local_tick_total);
  for (int i=0; i<nsamples; ++i, ++pos)
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
FileSource::~FileSource()
{
  if (buffer)
  {
    SDL_FreeWAV(buffer);
    buffer = nullptr;
  }
}

Dataflow::Module module
{
  "file",
  "File",
  "Audio File Input",
  "audio",
  {},  // no properties?
  {},  // no inputs
  { "Audio" }  // outputs
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(FileSource, module)
