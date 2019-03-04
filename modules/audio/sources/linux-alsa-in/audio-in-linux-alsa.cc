//==========================================================================
// ViGraph dataflow module:
//    audio/filters/linux-alsa-in/audio-in-linux-alsa.cc
//
// Source to capture audio from the Linux ALSA input
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../audio-module.h"
#include <alsa/asoundlib.h>

namespace {

using namespace ViGraph::Dataflow;
using namespace ViGraph::Module::Audio;

const auto default_device{"default"};

//==========================================================================
// LinuxAudioIn filter
class LinuxALSAInSource: public Source
{
  snd_pcm_t *pcm = nullptr;
  vector<Speaker> channel_mapping;

  // Read samples from device
  bool read_samples(vector<sample_t>& samples);

  // Source/Element virtuals
  void tick(const TickData& td) override;
  void shutdown() override;

public:
  LinuxALSAInSource(const Dataflow::Module *module,
                     const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML:
//   <audio-out/>
LinuxALSAInSource::LinuxALSAInSource(const Dataflow::Module *module,
                                       const XML::Element& config):
    Element(module, config), Source(module, config)
{
  Log::Streams log;
  const auto& device = config.get_attr("device", default_device);
  log.summary << "Opening audio input on ALSA device '" << device << "'\n";
  log.detail << "ALSA library version: " << SND_LIB_VERSION_STR << endl;

  auto nchannels = config.get_attr_int("channels", 2);

  const auto cmit = channel_mappings.find(nchannels);
  if (cmit == channel_mappings.end())
    throw runtime_error(string("Unsupported channel count: ") +
                        Text::itos(nchannels));
  channel_mapping = cmit->second;

  log.detail << "ALSA: " << nchannels << " channels\n";
  const auto start_threshold = config.get_attr_int("start-threshold", 1000);

  try
  {
    // Open PCM
    auto status = snd_pcm_open(&pcm, device.c_str(),
                               SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK);
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

    // Set up, configure and use swparams
    snd_pcm_sw_params_t *sw_params;
    snd_pcm_sw_params_alloca(&sw_params);

    status = snd_pcm_sw_params_current(pcm, sw_params);
    if (status < 0)
      throw runtime_error(string("sw_params:current: ")+snd_strerror(status));

    status = snd_pcm_sw_params_set_start_threshold(pcm, sw_params,
                                                   start_threshold);
    if (status < 0)
      throw runtime_error(string("sw_params:start_threshold: ")
                                 +snd_strerror(status));

    status = snd_pcm_sw_params(pcm, sw_params);
    if (status < 0)
      throw runtime_error(string("sw_params_set: ")+snd_strerror(status));

    // Prepare to send
    status = snd_pcm_prepare(pcm);
    if (status < 0)
      throw runtime_error(string("prepare: ")+snd_strerror(status));

    log.detail << "Created Linux ALSA audio in\n";
  }
  catch (runtime_error e)
  {
    log.error << "Can't open ALSA PCM input: " << e.what() << endl;
    if (pcm) snd_pcm_close(pcm);
    pcm = nullptr;
  }
}

//--------------------------------------------------------------------------
// Read samples from device
bool LinuxALSAInSource::read_samples(vector<sample_t>& samples)
{
  const auto samples_to_read = samples.size() / channel_mapping.size();
  auto n = snd_pcm_readi(pcm, &samples[0], samples_to_read);
  if (n < 0)
  {
    Log::Error log;
    log << "ALSA read error: " << snd_strerror(n) << endl;

    n = snd_pcm_recover(pcm, n, 1);
    if (!n)
      return false;
  }
  return true;
}

//--------------------------------------------------------------------------
// Read some data
void LinuxALSAInSource::tick(const TickData& td)
{
  const auto nsamples = td.samples(sample_rate);
  auto fragment = new Fragment(td.t);
  auto samples = vector<sample_t>(nsamples * channel_mapping.size());
  while (pcm)  // loop after recover
  {
    if (!read_samples(samples))
      continue;

    for (auto c = 0u; c < channel_mapping.size(); ++c)
    {
      auto& wc = fragment->waveforms[channel_mapping[c]];
      wc.reserve(samples.size() / channel_mapping.size());
      for (auto i = 0u; i < samples.size(); i += channel_mapping.size())
        wc.push_back(samples[i + c]);
    }
    break;
  }

  send(fragment);
}

//--------------------------------------------------------------------------
// Shut down
void LinuxALSAInSource::shutdown()
{
  if (pcm) snd_pcm_close(pcm);
  pcm = nullptr;
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "audio-in",
  "Audio input",
  "Audio input for Linux/ALSA",
  "audio",
  {
      { "device",  { {"Device to capture from", "default"}, Value::Type::text,
                                                        "@device" } },
      { "channels",  { {"Number of channels", "1"}, Value::Type::number,
                                                        "@number" } },
      { "start-threshold",
        { {"Number of samples to buffer before playback will start", "1000"},
           Value::Type::number, "@number" } }
  },
  { "Audio" }, // inputs
  { "Audio" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(LinuxALSAInSource, module)
