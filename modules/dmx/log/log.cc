//==========================================================================
// ViGraph dataflow module: dmx/log.cc
//
// Module which logs DMX states received
//
// Copyright (c) 2023 Paul Clark.  All rights reserved
//==========================================================================

#include "../dmx-module.h"

namespace {

using namespace ViGraph::Dataflow;
const auto default_frame_rate = 10;

//==========================================================================
// Log control
class LogControl: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;
  void setup(const SetupContext&) override;

  // Clone
  LogControl *create_clone() const override
  {
    return new LogControl{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Settings
  Setting<Number> frame_rate{default_frame_rate};

  // Input
  Input<DMX::State> input;
};

//--------------------------------------------------------------------------
// Setup
void LogControl::setup(const SetupContext& context)
{
  SimpleElement::setup(context);
  input.set_sample_rate(frame_rate);
}

//--------------------------------------------------------------------------
// Tick
void LogControl::tick(const TickData& td)
{
  Log::Detail log;
  const auto nsamples = td.samples_in_tick(frame_rate);
  sample_iterate(td, nsamples, {}, tie(input), {},
                 [&](const DMX::State& input)
  {
    log << input.get_as_json().str() << endl;
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "log",
  "DMX Log",
  "dmx",
  {
    { "frame-rate", &LogControl::frame_rate }
  },
  {
    { "input",   &LogControl::input   },
  },
  {
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(LogControl, module)
