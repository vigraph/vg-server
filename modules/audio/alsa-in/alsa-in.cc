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
const auto default_channels = 1;
const auto default_start_threshold = 2000;
const auto default_max_delay = 4000;
const auto default_max_recovery = 1;

//==========================================================================
// ALSA out
class ALSAIn: public DynamicElement
{
public:
  const static DynamicModule alsa_in_module;

private:
  snd_pcm_t *pcm = nullptr;
  vector<float> input_buffer;
  double input_sample_rate = default_sample_rate;
  double pos = 0.0;

  // Source/Element virtuals
  void setup(const SetupContext& context) override;
  void tick(const TickData& td) override;
  void shutdown() override;

  // Clone
  ALSAIn *create_clone() const override
  {
    return new ALSAIn{module};
  }

  vector<unique_ptr<Output<Number>>> outputs;

public:
  using DynamicElement::DynamicElement;

  Setting<string> device{default_device};
  Setting<Number> sample_rate{default_sample_rate};
  Setting<Number> nchannels{default_channels};
  Setting<Number> start_threshold{default_start_threshold};
  Setting<Number> max_delay{default_max_delay};
  Setting<Number> max_recovery{default_max_recovery};
};

//--------------------------------------------------------------------------
// Setup
void ALSAIn::setup(const SetupContext&)
{
  Log::Streams log;
  shutdown();

  log.summary << "Opening audio input on ALSA device '" << device << "'\n";
  log.detail << "ALSA library version: " << SND_LIB_VERSION_STR << endl;
  log.detail << "ALSA: " << nchannels << " channels\n";

  try
  {
    // Open PCM
    auto status = snd_pcm_open(&pcm, device.get().c_str(),
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
    input_sample_rate = srate;

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
    while (outputs.size() > nchannels)
    {
      module.erase_output("channel" + Text::itos(outputs.size()));
      outputs.pop_back();
    }
    while (outputs.size() < nchannels)
    {
      outputs.emplace_back(new Output<Number>{});
      module.add_output("channel" + Text::itos(outputs.size()),
                        outputs.back().get());
    }

    log.detail << "Created ALSA audio in\n";
  }
  catch (const runtime_error& e)
  {
    log.error << "Can't open ALSA audio input: " << e.what() << endl;
  }
}

//--------------------------------------------------------------------------
// Process some data
void ALSAIn::tick(const TickData& td)
{
  if (pcm)
  {
    pos = fmod(pos, 1);
    auto sample_rate = double{};
    auto obuffers = vector<Output<Number>::Buffer>{};
    obuffers.reserve(outputs.size());
    for (auto& o: outputs)
    {
      if (o->get_sample_rate() > sample_rate)
        sample_rate = o->get_sample_rate();
      obuffers.emplace_back(o->get_buffer(td));
    }
    const auto nsamples = td.samples_in_tick(sample_rate);
    const auto in_nsamples = td.samples_in_tick(input_sample_rate);
    const auto step = input_sample_rate / sample_rate;
    const auto channels = nchannels.get();
    input_buffer.resize(in_nsamples * channels);

    auto n = snd_pcm_readi(pcm, input_buffer.data(),
                           input_buffer.size() / channels);
    if (n < 0)
    {
      Log::Error log;
      log << "ALSA read error: " << snd_strerror(n) << endl;

      n = snd_pcm_recover(pcm, n, 1);
    }

    for (auto i = 0u; i < nsamples; ++i)
    {
      const auto p = fmod(pos, 1);
      const auto i1 = static_cast<unsigned>(pos);
      const auto i2 = i1 + 1;
      auto c = 0;
      for (auto& o: obuffers)
      {
        const auto c1 = i1 * channels + c;
        const auto c2 = i2 * channels + c;
        const auto s1 = c1 < n ? input_buffer[c1] : 0.0;
        const auto s2 = c2 < n ? input_buffer[c2] : s1;
        o.data.emplace_back(s1 + ((s2 - s1) * p));
        ++c;
      }
      pos += step;
    }
  }
}

//--------------------------------------------------------------------------
// Shut down
void ALSAIn::shutdown()
{
  if (pcm) snd_pcm_close(pcm);
  pcm = nullptr;
}

//--------------------------------------------------------------------------
// Module definition
const DynamicModule ALSAIn::alsa_in_module =
{
  "alsa-in",
  "ALSA Audio input",
  "audio",
  {
    { "device", &ALSAIn::device },
    { "sample-rate", &ALSAIn::sample_rate },
    { "channels", &ALSAIn::nchannels },
    { "start-threshold", &ALSAIn::start_threshold },
    { "max-delay", &ALSAIn::max_delay },
    { "max-recovery", &ALSAIn::max_recovery },
  },
  {},
  {
    // dynamic output channels
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ALSAIn, ALSAIn::alsa_in_module)

