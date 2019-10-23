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

  // Clone
  LogControl *create_clone() const override
  {
    return new LogControl{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Settings
  Setting<string> prefix;  // Default empty

  // Input
  Input<double> input;
};

//--------------------------------------------------------------------------
// Tick
void LogControl::tick(const TickData& td)
{
  sample_iterate(td.nsamples, {}, tie(input), {},
                 [&](double input)
  {
    if (input != last_input)
    {
      Log::Detail log;
      log << prefix << input << endl;
      last_input = input;
    }
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "log",
  "Log",
  "core",
  {
    { "prefix", &LogControl::prefix }
  },
  {
    { "input",  &LogControl::input  },
  },
  {
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(LogControl, module)
