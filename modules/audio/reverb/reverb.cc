//==========================================================================
// ViGraph dataflow module: audio/reverb/reverb.cc
//
// Reverb module
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../audio-module.h"

namespace {

//==========================================================================
// Reverb
class Reverb: public SimpleElement
{
private:
  deque<double> buffer;

  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Reverb *create_clone() const override
  {
    return new Reverb{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Input<double> input{0.0};
  Input<double> time{0.0};
  Input<double> feedback{0.0};
  Output<double> output;
};

//--------------------------------------------------------------------------
// Tick data
void Reverb::tick(const TickData& td)
{
  const auto sample_rate = output.get_sample_rate();

  const auto nsamples = td.samples_in_tick(sample_rate);

  sample_iterate(nsamples, {}, tie(input, time, feedback), tie(output),
                 [&](double i, double t, double f, double& o)
  {
    const auto bsamples = static_cast<size_t>(t * sample_rate);
    if (bsamples > buffer.size())
    {
      buffer.insert(buffer.begin(), bsamples - buffer.size(),
                    buffer.empty() ? 0 : buffer.front());
    }
    else if (bsamples < buffer.size())
    {
      buffer.resize(bsamples);
    }
    if (bsamples)
    {
      o = i + buffer.front() * f;
      buffer.pop_front();
      buffer.emplace_back(o);
    }
    else
    {
      o = i;
    }
  });
}

Dataflow::SimpleModule module
{
  "reverb",
  "Reverb",
  "audio",
  {},
  {
    { "input",    &Reverb::input },
    { "time",     &Reverb::time },
    { "feedback", &Reverb::feedback },
  },
  {
    { "output",   &Reverb::output }
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Reverb, module)