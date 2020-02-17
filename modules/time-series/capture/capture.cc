//==========================================================================
// ViGraph dataflow module: time-series/capture/capture.cc
//
// Capture a time series from a number input
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../time-series-module.h"
#include <cmath>
#include <cfloat>

namespace {

const auto default_sample_rate = 1000;
const auto default_samples = 1000;

//==========================================================================
// Capture
class Capture: public SimpleElement
{
private:
  vector<Number> buffer;
  bool running{false};

  // Element virtuals
  void setup(const SetupContext& context) override;
  void update_sample_rate() override {}
  void tick(const TickData& td) override;

  // Clone
  Capture *create_clone() const override
  {
    return new Capture{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Setting<Number> sample_rate{default_sample_rate};
  Setting<Integer> samples{default_samples};
  Setting<bool> rotate{false};

  // Input
  Input<Number> input;
  Input<Trigger> start;
  Input<Trigger> stop;
  Input<Trigger> clear;

  // Output
  Output<DataCollection> output;
};

//--------------------------------------------------------------------------
// Setup
void Capture::setup(const SetupContext& context)
{
  SimpleElement::setup(context);
  input.set_sample_rate(sample_rate);
  start.set_sample_rate(sample_rate);
  stop.set_sample_rate(sample_rate);
  clear.set_sample_rate(sample_rate);
}

//--------------------------------------------------------------------------
// Tick data
void Capture::tick(const TickData& td)
{
  // Check trigger inputs in parallel to preserve ordering
  const auto& cl = clear.get_buffer();
  const auto& sta = start.get_buffer();
  const auto& sto = start.get_buffer();
  const auto insamples = max(cl.size(), max(sta.size(), sto.size()))+1;
                                                            // In case all 0
  sample_iterate(td, insamples, {}, tie(clear, start, stop), {},
                 [&](Trigger _clear, Trigger _start, Trigger _stop)
  {
    if (_clear)
    {
      buffer.clear();
      running = false;
    }

    if (_start || !start.connected())
      running = true;

    if (_stop) running = false;  // Note allow stop even if start NC
  });

  // Capture input into buffer, if running
  if (running && samples)
  {
    const auto& in = input.get_buffer();
    buffer.insert(buffer.end(), in.begin(), in.end());

    if (buffer.size() >= (size_t)samples)
    {
      if (rotate)
      {
        // Prune from front
        buffer.erase(buffer.begin(), buffer.begin()+(buffer.size()-samples));
      }
      else
      {
        // Throw away excess and stop
        buffer.erase(buffer.begin()+samples, buffer.end());
        running = false;
      }
    }
  }

  // Construct the dataset
  DataSet data;
  auto sample_time = td.first_sample_at(sample_rate);
  const auto sample_duration = td.sample_duration(sample_rate);
  for(auto i=0u; i<buffer.size(); i++)
  {
    data.add(sample_time, buffer[i]);
    sample_time += sample_duration;
  }

  // Copy to all output samples
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, {}, tie(output),
                 [&](DataCollection& output)
  {
    output.add(data);
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "capture",
  "Capture",
  "time-series",
  {
    { "sample-rate",  &Capture::sample_rate },
    { "samples",      &Capture::samples },
    { "rotate",       &Capture::rotate }
  },
  {
    { "input",        &Capture::input },
    { "start",        &Capture::start },
    { "stop",         &Capture::stop  },
    { "clear",        &Capture::clear }
  },
  {
    { "output",       &Capture::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Capture, module)
