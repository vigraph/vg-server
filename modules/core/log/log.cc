//==========================================================================
// ViGraph dataflow module: log/log.cc
//
// Module which logs values received
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module.h"
#include <cmath>

namespace {

//==========================================================================
// Log control
class LogControl: public SimpleElement
{
private:
  double last_input{0.0};

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
  Setting<string> text;  // Default empty

  // Input
  Input<Number> input;
  Input<Trigger> trigger;
};

//--------------------------------------------------------------------------
// Setup
void LogControl::setup(const SetupContext&)
{
  input.set_sample_rate(25.0);  // Arbitrary, but usable
  trigger.set_sample_rate(25.0);
}

//--------------------------------------------------------------------------
// Tick
void LogControl::tick(const TickData&)
{
  // Run two loops in parallel, being aware that they might not both have
  // samples
  const auto& inputs = input.get_buffer();
  const auto& triggers = trigger.get_buffer();
  auto n = max(inputs.size(), triggers.size());

  for(auto i=0u; i<n; i++)
  {
    if (i<inputs.size() && inputs[i] != last_input)
    {
      Log::Detail log;
      log << text << inputs[i] << endl;
      last_input = inputs[i];
    }

    if (i<triggers.size() && triggers[i])
    {
      Log::Detail log;
      log << text << endl;
    }
  }
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "log",
  "Log",
  "core",
  {
    { "text", &LogControl::text }
  },
  {
    { "input",   &LogControl::input   },
    { "trigger", &LogControl::trigger }
  },
  {
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(LogControl, module)
