//==========================================================================
// ViGraph dataflow module: controls/key-frame/key-frame.cc
//
// Control to alter one or more properties through key-frame interpolation
// over time
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include <cmath>

namespace {

//==========================================================================
// KeyFrame control
class KeyFrameControl: public Control
{
  // Configured state
  bool wait{false};

  // Dynamic state
  bool triggered{false};

  // Control virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void tick(const TickData& td) override;
  // Automatically set wait flag if we are the target of something
  void notify_target_of(Element *) override { wait = true; }
  void enable() override;

public:
  // Construct
  KeyFrameControl(const Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
// <key-frame property="...">
//   <at time="0" x="0"/>
//   <at time="1" x="0.5"/>
// <key-frame>
KeyFrameControl::KeyFrameControl(const Module *module,
                                 const XML::Element& config):
  Element(module, config), Control(module, config)
{
  wait = config.get_attr_bool("wait");
}

//--------------------------------------------------------------------------
// Set a control property
void KeyFrameControl::set_property(const string& property, const SetParams&)
{
  if (property == "trigger")
    triggered = true;
}

//--------------------------------------------------------------------------
// Enable (reset)
void KeyFrameControl::enable()
{
  triggered = false;
}

//--------------------------------------------------------------------------
// Tick
void KeyFrameControl::tick(const TickData& /*td*/)
{
  if (wait)
  {
    if (!triggered) return;
    triggered = false;
  }
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "key-frame",
  "KeyFrame",
  "Animate a property using key frame interpolation",
  "core",
  {
    { "wait",  { "Whether to wait for a trigger", Value::Type::number } },
    { "trigger", { "Trigger to set value", Value::Type::trigger, true } }
  },
  { { "", { "Any value", "", Value::Type::any }}} // Flexible controlled property
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(KeyFrameControl, module)
