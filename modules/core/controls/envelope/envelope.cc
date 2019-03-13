//==========================================================================
// ViGraph dataflow module: controls/envelope/envelope.cc
//
// Control to output an ADSR envelope value on trigger/release
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include <iostream>

namespace {

//==========================================================================
// Envelope control
class EnvelopeControl: public Dataflow::Control
{
  // Configured state
  timestamp_t attack{0.0};   // Time for attack
  timestamp_t decay{0.0};    // Time for decay
  double sustain{1.0};       // Value to sustain at
  timestamp_t release{0.0};  // Time to release

  // Dynamic state
  enum class State
  {
    off,
    attack,
    decay,
    sustain,
    release,
    complete
  } state = State::off;
  bool state_changed{false};
  timestamp_t state_changed_time{0};
  double release_start_value{0};
  double attack_start_value{0};

  // Control virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void pre_tick(const TickData& td) override;
  void enable() override;

public:
  // Construct
  EnvelopeControl(const Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
// <envelope attack="0.0" decay="0.0" sustain="1.0" release="0.0"
//      property="..."/>
// Default are hard on/off, no decay
EnvelopeControl::EnvelopeControl(const Module *module,
                                 const XML::Element& config):
  Control(module, config)
{
  attack = config.get_attr_real("attack", 0.0);
  decay = config.get_attr_real("decay", 0.0);
  sustain = config.get_attr_real("sustain", 1.0);
  release = config.get_attr_real("release", 0.0);
}

//--------------------------------------------------------------------------
// Set a control property
void EnvelopeControl::set_property(const string& property, const SetParams& sp)
{
  if (property == "trigger")
  {
    state = State::attack;  // Note whatever previous state was, to restart
    state_changed = true;
    send("trigger", Dataflow::Value{});
  }
  else if (property == "clear")
  {
    switch (state)
    {
      case State::attack:
      case State::decay:
      case State::sustain:
        state = State::release;
        state_changed = true;
        // In case of immediate attack before next tick
        attack_start_value = release_start_value;
        break;

      default:;
    }
  }
  else if (property == "attack")
    update_prop(attack, sp);
  else if (property == "decay")
    update_prop(decay, sp);
  else if (property == "sustain")
    update_prop(sustain, sp);
  else if (property == "release")
    update_prop(release, sp);
}

//--------------------------------------------------------------------------
// Enable (reset)
void EnvelopeControl::enable()
{
  state = State::off;
  state_changed = false;
  attack_start_value = 0;
}

//--------------------------------------------------------------------------
// Tick
void EnvelopeControl::pre_tick(const TickData& td)
{
  // Check for state change
  if (state_changed)
  {
    state_changed_time = td.t;
    state_changed = false;
  }

  double value;
  timestamp_t delta = td.t-state_changed_time;
  switch (state)
  {
    case State::off:
      attack_start_value = release_start_value = value = 0;
      return;

    case State::attack:
      if (delta >= attack)
      {
        value = 1.0;
        state = State::decay;
        state_changed_time = td.t;
      }
      else
      {
        value = attack_start_value + (1-attack_start_value)*delta/attack;
      }
      // In case release while attacking
      release_start_value = value;
    break;

    case State::decay:
      if (delta >= decay)
      {
        value = sustain;
        state = State::sustain;
        state_changed_time = td.t;
      }
      else
      {
        value = 1.0-(1.0-sustain)*delta/decay;
      }
      // In case either release or new trigger while attacking
      attack_start_value = release_start_value = value;
    break;

    case State::sustain:
      attack_start_value = release_start_value = value = sustain;
    break;

    case State::release:
      if (delta >= release)
      {
        value = 0;
        state = State::complete;
        state_changed_time = td.t;
      }
      else
      {
        value = release_start_value*(1.0-delta/release);
      }
      attack_start_value = value;
    break;

    case State::complete:
      send("clear", Dataflow::Value{});
      state = State::off;
      return;
  }

  send("", Dataflow::Value{value});
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "envelope",
  "Envelope",
  "Provide an ADSR envelope on trigger/release",
  "core",
  {
    { "attack",  { "Attack time", Value::Type::number, true } },
    { "decay",   { "Decay time", Value::Type::number, true } },
    { "sustain", { "Sustain level", Value::Type::number, true } },
    { "release", { "Release time", Value::Type::number, true } },
    { "trigger", { "Trigger to start attack", Value::Type::trigger, true } },
    { "clear",   { "Trigger to start release", Value::Type::trigger, true } }
  },
  {
    { "", { "Envelope value", "", Value::Type::number }},
    { "trigger", { "Envelope started", "trigger", Value::Type::trigger } },
    { "clear",   { "Envelope complete", "clear", Value::Type::trigger } }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(EnvelopeControl, module)
