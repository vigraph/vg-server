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

const auto default_device{"default"};

//==========================================================================
// LinuxAudioOut filter
class LinuxALSAOutFilter: public FragmentFilter
{
  snd_pcm_t *pcm{nullptr};
  int nchannels{2};

  // Source/Element virtuals
  void accept(FragmentPtr fragment) override;
  void shutdown() override;

public:
  LinuxALSAOutFilter(const Dataflow::Module *module,
                     const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML:
//   <audio-out/>
LinuxALSAOutFilter::LinuxALSAOutFilter(const Dataflow::Module *module,
                                       const XML::Element& config):
    Element(module, config), FragmentFilter(module, config)
{
  Log::Streams log;
  const auto& device = config.get_attr("device", default_device);
  log.summary << "Opening audio output on ALSA device '" << device << "'\n";
  log.detail << "ALSA library version: " << SND_LIB_VERSION_STR << endl;

  nchannels = config.get_attr_int("channels", 2);
  log.detail << "ALSA: " << nchannels << " channels\n";

  try
  {
    // Open PCM
    auto status = snd_pcm_open(&pcm, device.c_str(),
                               SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
    if (status < 0)
      throw runtime_error(string("open: ")+snd_strerror(status));

    // Set up, configure and use hwparams
    snd_pcm_hw_params_t *hw_params;
    snd_pcm_hw_params_alloca(&hw_params);

    status = snd_pcm_hw_params_any(pcm, hw_params);
    if (status < 0)
      throw runtime_error(string("hw_params_any: ")+snd_strerror(status));

    status = snd_pcm_hw_params_set_access(pcm, hw_params,
                                          SND_PCM_ACCESS_RW_INTERLEAVED);
    if (status < 0)
      throw runtime_error(string("hw_params_access: ")+snd_strerror(status));

    status = snd_pcm_hw_params_set_format(pcm, hw_params, SND_PCM_FORMAT_FLOAT);
    if (status < 0)
      throw runtime_error(string("hw_params_format: ")+snd_strerror(status));

    unsigned int srate = sample_rate;
    status = snd_pcm_hw_params_set_rate_near(pcm, hw_params, &srate, 0);
    if (status < 0)
      throw runtime_error(string("hw_params_rate: ")+snd_strerror(status));
    log.detail << "ALSA: sample rate chosen " << srate << endl;

    status = snd_pcm_hw_params_set_channels(pcm, hw_params, nchannels);
    if (status < 0)
      throw runtime_error(string("hw_params:channels: ")+snd_strerror(status));

    status = snd_pcm_hw_params(pcm, hw_params);
    if (status < 0)
      throw runtime_error(string("hw_params_set: ")+snd_strerror(status));

    // Prepare to send
    status = snd_pcm_prepare(pcm);
    if (status < 0)
      throw runtime_error(string("prepare: ")+snd_strerror(status));

    log.detail << "Created Linux ALSA audio out\n";
  }
  catch (runtime_error e)
  {
    log.error << "Can't open ALSA PCM output: " << e.what() << endl;
    if (pcm) snd_pcm_close(pcm);
    pcm = nullptr;
  }
}

//--------------------------------------------------------------------------
// Process some data
void LinuxALSAOutFilter::accept(FragmentPtr fragment)
{
  while (pcm && fragment->nchannels == nchannels)  // loop after recover
  {
    // Send out the fragment
    ssize_t n = snd_pcm_writei(pcm, fragment->waveform.data(),
                               fragment->waveform.size()/fragment->nchannels);
    if (n < 0)
    {
      Log::Error log;
      log << "ALSA write error: " << snd_strerror(n) << endl;

      n = snd_pcm_recover(pcm, n, 1);
      if (!n) continue;  // retry
    }
    break;
  }

  // Send it down as well, so these can be chained
  send(fragment);
}

//--------------------------------------------------------------------------
// Shut down
void LinuxALSAOutFilter::shutdown()
{
  if (pcm) snd_pcm_close(pcm);
  pcm = nullptr;
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "audio-out",
  "Audio output",
  "Audio output for Linux/ALSA",
  "audio",
  {
      { "device",  { {"Device output to", "default"}, Value::Type::text,
                                                        "@device" } },
      { "channels",  { {"Number of channels", "1"}, Value::Type::number,
                                                        "@number" } }
  },
  { "Audio" }, // inputs
  { "Audio" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(LinuxALSAOutFilter, module)

