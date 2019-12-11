//==========================================================================
// ViGraph dataflow module: audio/delay/delay.cc
//
// Delay module
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../audio-module.h"

namespace {

//==========================================================================
// Delay
class Delay: public SimpleElement
{
private:
  deque<double> buffer;

  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Delay *create_clone() const override
  {
    return new Delay{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Input<double> input{0.0};
  Input<double> time{0.0};
  Output<double> output;
};

//--------------------------------------------------------------------------
// Tick data
void Delay::tick(const TickData& td)
{
  const auto sample_rate = output.get_sample_rate();

  const auto nsamples = td.samples_in_tick(sample_rate);

  sample_iterate(td, nsamples, {}, tie(input, time), tie(output),
                 [&](double i, double t, double& o)
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
      buffer.emplace_back(i);
      o = buffer.front();
      buffer.pop_front();
    }
    else
    {
      o = i;
    }
  });
}

Dataflow::SimpleModule module
{
  "delay",
  "Delay",
  "audio",
  {},
  {
    { "input",  &Delay::input },
    { "time",   &Delay::time }
  },
  {
    { "output", &Delay::output }
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Delay, module)
