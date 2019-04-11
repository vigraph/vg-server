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
const auto default_channels = 2;
const auto default_start_threshold = 1000;

//==========================================================================
// LinuxAudioOut filter
class LinuxALSAOutFilter: public FragmentFilter
{
public:
  string device = default_device;
  int nchannels = default_channels;
  int start_threshold = default_start_threshold;

private:
  snd_pcm_t *pcm{nullptr};
  unsigned long tick_samples_required = 0;
  vector<sample_t> output_buffer;

  // Write samples to device
  bool write_samples(const vector<sample_t>& samples);

  // Source/Element virtuals
  void setup() override;
  void pre_tick(const TickData& td) override;
  void accept(FragmentPtr fragment) override;
  void post_tick(const TickData& td) override;
  void shutdown() override;

public:
  using FragmentFilter::FragmentFilter;
};

//--------------------------------------------------------------------------
// Setup
void LinuxALSAOutFilter::setup()
{
  Log::Streams log;
  log.summary << "Opening audio output on ALSA device '" << device << "'\n";
  log.detail << "ALSA library version: " << SND_LIB_VERSION_STR << endl;
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
// Write samples to device
bool LinuxALSAOutFilter::write_samples(const vector<sample_t>& samples)
{
  const auto samples_to_write = min(tick_samples_required,
                                    static_cast<unsigned long>(samples.size()
                                                               / nchannels));
  auto n = snd_pcm_writei(pcm, samples.data(), samples_to_write);
  if (n < 0)
  {
    Log::Error log;
    log << "ALSA write error: " << snd_strerror(n) << endl;

    n = snd_pcm_recover(pcm, n, 1);
    if (!n)
      return false;
  }
  else
  {
    tick_samples_required -= min(static_cast<unsigned long>(n),
                                 tick_samples_required);
  }
  return true;
}

//--------------------------------------------------------------------------
// Process some data
void LinuxALSAOutFilter::accept(FragmentPtr fragment)
{
  auto& waveforms = fragment->waveforms;

  // Find max samples per waveform
  auto num_samples = 0ul;
  for (auto& waveform: waveforms)
    num_samples = max(waveform.second.size(), num_samples);

  // Cap samples at tick requirement
  num_samples = min(num_samples, tick_samples_required);

  // Build an interleaved output buffer
  output_buffer.resize(num_samples * nchannels);
  fill(output_buffer.begin(), output_buffer.end(), 0.0);
  auto wit = waveforms.begin();
  for (auto i = 0; i < nchannels; ++i)
  {
    if (wit == waveforms.end())
      break;
    for (auto s = 0ul; s < num_samples; ++s)
    {
      if (s >= wit->second.size())
        break;
      output_buffer[s * nchannels + i] = wit->second[s];
    }
    ++wit;
  }

  // Try and write the samples
  while (pcm)  // loop after recover
  {
    if (!write_samples(output_buffer))
      continue;
    break;
  }

  // Send it down as well, so these can be chained
  send(fragment);
}

//--------------------------------------------------------------------------
// Record how many samples we need for tick
void LinuxALSAOutFilter::pre_tick(const TickData &td)
{
  tick_samples_required = td.samples();
}

//--------------------------------------------------------------------------
// If tick has not been completely fulfilled, fill in with empty data to avoid
// underruns
void LinuxALSAOutFilter::post_tick(const TickData &)
{
  if (tick_samples_required)
  {
    output_buffer.resize(nchannels * tick_samples_required);
    fill(output_buffer.begin(), output_buffer.end(), 0);
    while (pcm)  // loop after recover
    {
      if (!write_samples(output_buffer))
        continue;
      break;
    }
  }
  tick_samples_required = 0;
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
      { "device", { "Device output to", Value::Type::text,
                    &LinuxALSAOutFilter::device, false } },
      { "channels", { "Number of channels", Value::Type::number,
                      &LinuxALSAOutFilter::nchannels, false } },
      { "start-threshold",
        { "Number of samples to buffer before playback will start",
          Value::Type::number,
          &LinuxALSAOutFilter::start_threshold, false } },
  },
  { "Audio" }, // inputs
  { "Audio" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(LinuxALSAOutFilter, module)

