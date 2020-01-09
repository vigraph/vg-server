//==========================================================================
// Waveform Switch module
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../waveform-module.h"
#include "../../switch.h"

template<>
inline Waveform::Type switch_fade(const Waveform::Type& value, double factor)
{
  if (factor)
    return value;
  else
    return Waveform::Type::none;
}

namespace {

class WaveformSwitch: public Switch<Waveform::Type>
{
public:
  const static Dataflow::DynamicModule switch_module;

private:
  // Clone
  WaveformSwitch *create_clone() const override
  {
    return new WaveformSwitch{switch_module};
  }
public:
  using Switch::Switch;
};

const Dataflow::DynamicModule WaveformSwitch::switch_module =
{
  "switch",
  "Switch",
  "waveform",
  {
    { "inputs",         &WaveformSwitch::inputs },
  },
  {
    { "number",         &WaveformSwitch::number },
    { "fraction",       &WaveformSwitch::fraction },
    { "next",           &WaveformSwitch::next },
  },
  {
    { "output",         &WaveformSwitch::output },
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(WaveformSwitch,
                                   WaveformSwitch::switch_module)
