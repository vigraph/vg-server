//==========================================================================
// ViGraph dataflow module: audio/alsa-out/alsa-out.cc
//
// Output audio to the ALSA output
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../audio-module.h"
#include <alsa/asoundlib.h>

namespace {

using namespace ViGraph::Dataflow;

const auto default_device{"default"};
const auto default_sample_rate{44100};
const auto default_channels = 2;
const auto default_start_threshold = 2000;

//==========================================================================
// ALSA out
class ALSAOut: public SimpleElement
{
private:
  snd_pcm_t *pcm = nullptr;
  vector<float> output_buffer;

  // Source/Element virtuals
  void setup(const SetupContext& context) override;
  void tick(const TickData& td) override;
  void shutdown() override;

  // Clone
  ALSAOut *create_clone() const override
  {
    return new ALSAOut{module};
  }

public:
  using SimpleElement::SimpleElement;

  Setting<string> device{default_device};
  Setting<Number> sample_rate{default_sample_rate};
  Setting<Number> nchannels{default_channels};
  Setting<Number> start_threshold{default_start_threshold};

  Input<AudioData> input;
};

//--------------------------------------------------------------------------
// Setup
void ALSAOut::setup(const SetupContext& context)
{
  SimpleElement::setup(context);

  Log::Streams log;
  shutdown();

  log.summary << "Opening audio output on ALSA device '" << device << "'\n";
  log.detail << "ALSA library version: " << SND_LIB_VERSION_STR << endl;
  log.detail << "ALSA: " << nchannels << " channels\n";

  try
  {
    // Open PCM
    auto status = snd_pcm_open(&pcm, device.get().c_str(),
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

    if (nchannels > max_channels)
      throw runtime_error("Too many channels in ALSA output");
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

    input.set_sample_rate(srate);

    log.detail << "Created ALSA audio out\n";
  }
  catch (const runtime_error& e)
  {
    log.error << "Can't open ALSA audio output: " << e.what() << endl;
  }
}

//--------------------------------------------------------------------------
// Process some data
void ALSAOut::tick(const TickData& td)
{
  if (pcm)
  {
    const auto nsamples = td.samples_in_tick(input.get_sample_rate());
    const auto channels = nchannels;
    output_buffer.clear();
    output_buffer.reserve(nsamples * channels);
    sample_iterate(td, nsamples, {},
                   tie(input), {},
                   [&](const AudioData& input)
    {
      for(auto c=0; c<input.nchannels; c++)
        output_buffer.emplace_back(input.channels[c]);

      // Zero any we don't get
      for(auto c=input.nchannels; c<channels; c++)
        output_buffer.emplace_back(0);
    });

    auto n = snd_pcm_writei(pcm, output_buffer.data(),
                            output_buffer.size() / channels);
    if (n < 0)
    {
      Log::Error log;
      log << "ALSA write error: " << snd_strerror(n) << endl;

      n = snd_pcm_recover(pcm, n, 1);
    }
  }
}

//--------------------------------------------------------------------------
// Shut down
void ALSAOut::shutdown()
{
  if (pcm) snd_pcm_close(pcm);
  pcm = nullptr;
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "alsa-out",
  "ALSA Audio output",
  "audio",
  {
    { "device", &ALSAOut::device },
    { "sample-rate", &ALSAOut::sample_rate },
    { "channels", &ALSAOut::nchannels },
    { "start-threshold", &ALSAOut::start_threshold },
  },
  {
    { "input", &ALSAOut::input }
  },
  {}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ALSAOut, module)

