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
const auto default_max_delay = 4000;
const auto default_max_recovery = 1;

//==========================================================================
// ALSA out
class ALSAOut: public DynamicElement
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
  using DynamicElement::DynamicElement;

  Setting<string> device{default_device};
  Setting<double> sample_rate{default_sample_rate};
  Setting<double> nchannels{default_channels};
  Setting<double> start_threshold{default_start_threshold};
  Setting<double> max_delay{default_max_delay};
  Setting<double> max_recovery{default_max_recovery};

  Input<double> channel1;
  Input<double> channel2;
  Input<double> channel3;
  Input<double> channel4;
  Input<double> channel5;
  Input<double> channel6;
};

//--------------------------------------------------------------------------
// Setup
void ALSAOut::setup(const SetupContext&)
{
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

    // Update module information
    if (nchannels < 6 && module.num_inputs() >= 6)
      module.erase_input("channel6");
    if (nchannels < 5 && module.num_inputs() >= 5)
      module.erase_input("channel5");
    if (nchannels < 4 && module.num_inputs() >= 4)
      module.erase_input("channel4");
    if (nchannels < 3 && module.num_inputs() >= 3)
      module.erase_input("channel3");
    if (nchannels < 2 && module.num_inputs() >= 2)
      module.erase_input("channel2");
    if (nchannels < 1 && module.num_inputs() >= 1)
      module.erase_input("channel1");
    if (nchannels >= 1 && module.num_inputs() < 1)
      module.add_input("channel1", &ALSAOut::channel1);
    if (nchannels >= 2 && module.num_inputs() < 2)
      module.add_input("channel2", &ALSAOut::channel2);
    if (nchannels >= 3 && module.num_inputs() < 3)
      module.add_input("channel3", &ALSAOut::channel3);
    if (nchannels >= 4 && module.num_inputs() < 4)
      module.add_input("channel4", &ALSAOut::channel4);
    if (nchannels >= 5 && module.num_inputs() < 5)
      module.add_input("channel5", &ALSAOut::channel5);
    if (nchannels >= 6 && module.num_inputs() < 6)
      module.add_input("channel6", &ALSAOut::channel6);

    channel1.set_sample_rate(srate);
    channel2.set_sample_rate(srate);
    channel3.set_sample_rate(srate);
    channel4.set_sample_rate(srate);
    channel5.set_sample_rate(srate);
    channel6.set_sample_rate(srate);

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
    const auto nsamples = td.samples_in_tick(channel1.get_sample_rate());
    const auto channels = nchannels;
    output_buffer.clear();
    output_buffer.reserve(nsamples * channels);
    sample_iterate(td, nsamples, {},
                   tie(channel1, channel2, channel3,
                       channel4, channel5, channel6), {},
                   [&](double c1, double c2, double c3,
                       double c4, double c5, double c6)
    {
      if (channels >= 1)
        output_buffer.emplace_back(c1);
      if (channels >= 2)
        output_buffer.emplace_back(c2);
      if (channels >= 3)
        output_buffer.emplace_back(c3);
      if (channels >= 4)
        output_buffer.emplace_back(c4);
      if (channels >= 5)
        output_buffer.emplace_back(c5);
      if (channels >= 6)
        output_buffer.emplace_back(c6);
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
Dataflow::DynamicModule module
{
  "alsa-out",
  "ALSA Audio output",
  "audio",
  {
    { "device", &ALSAOut::device },
    { "sample-rate", &ALSAOut::sample_rate },
    { "channels", &ALSAOut::nchannels },
    { "start-threshold", &ALSAOut::start_threshold },
    { "max-delay", &ALSAOut::max_delay },
    { "max-recovery", &ALSAOut::max_recovery },
  },
  {
    // dynamic input channels
  },
  {}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ALSAOut, module)

