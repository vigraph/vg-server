//==========================================================================
// ViGraph dataflow module: controls/log/log.cc
//
// Dummy control which logs a message on enabling
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include <cmath>

namespace {

//==========================================================================
// Log control
class LogControl: public Control
{
  // Control virtuals
  void enable();

public:
  string message;

  // Construct
  using Control::Control;
};

//--------------------------------------------------------------------------
// Enable
void LogControl::enable()
{
  Log::Summary log;
  log << "<log>: " << message << endl;
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "log",
  "Log",
  "Log a message on enabling",
  "core",
  {
    { "message", { "Message to log", Value::Type::text,
          &LogControl::message, true } }
  },
  {}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(LogControl, module)
