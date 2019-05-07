//==========================================================================
// ViGraph dataflow module: ui/controls/key/key.cc
//
// Generic UI key input control
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include "../../ui-services.h"

using namespace ViGraph::Module::UI;

namespace {

//--------------------------------------------------------------------------
// Key name map
const map<string, int> key_name_to_code{
  { "backspace",       8 },
  { "tab",             9 },
  { "enter",          13 },
  { "shift",          16 },
  { "ctrl",           17 },
  { "alt",            18 },
  { "pause",          19 },
  { "caps-lock",      20 },
  { "escape",         27 },
  { "page-up",        33 },
  { "page-down",      34 },
  { "end",            35 },
  { "home",           36 },
  { "left",           37 },
  { "up",             38 },
  { "right",          39 },
  { "down",           40 },
  { "insert",         45 },
  { "delete",         46 },
  { "win-left",       91 },
  { "win-right",      92 },
  { "select",         93 },
  { "num-0",          96 },
  { "num-1",          97 },
  { "num-2",          98 },
  { "num-3",          99 },
  { "num-4",         100 },
  { "num-5",         101 },
  { "num-6",         102 },
  { "num-7",         103 },
  { "num-8",         104 },
  { "num-9",         105 },
  { "num-star",      106 },
  { "num-plus",      107 },
  { "num-dash",      109 },
  { "num-dot",       110 },
  { "num-slash",     111 },
  { "f1",            112 },
  { "f2",            113 },
  { "f3",            114 },
  { "f4",            115 },
  { "f5",            116 },
  { "f6",            117 },
  { "f7",            118 },
  { "f8",            119 },
  { "f9",            120 },
  { "f10",           121 },
  { "f11",           122 },
  { "f12",           123 },
  { "num-lock",      144 },
  { "scroll-lock",   145 },
  { "semi",          186 },
  { "equal",         187 },
  { "comma",         188 },
  { "dash",          189 },
  { "dot",           190 },
  { "slash",         191 },
  { "back-tick",     192 },
  { "open-bracket",  219 },
  { "back-slash",    220 },
  { "close-bracket", 221 },
  { "quote",         222 }
};

//==========================================================================
// UIKey control
class UIKeyControl: public Dataflow::Control,
                    public KeyDistributor::KeyObserver
{
private:
  int code{0};
  enum class When
  {
    pressed,
    released,
  } when = When::pressed;

  // Control virtuals
  void enable() override;
  void disable() override;

  // Event observer implementation
  void handle_key(int code) override;

public:
  using Control::Control;

  // Property getter/setters
  string get_code() const;
  void set_code(const string& code);
  string get_when() const;
  void set_when(const string& when);
};

//--------------------------------------------------------------------------
// Enable - register for events
void UIKeyControl::enable()
{
  auto distributor =
    graph->find_service<KeyDistributor>("ui", "key-distributor");
  if (distributor)
  {
    distributor->register_key_observer(when == When::released ? -code : code,
                                       this);
  }
}

//--------------------------------------------------------------------------
// Disable (deregister for keys)
void UIKeyControl::disable()
{
  auto distributor =
    graph->find_service<KeyDistributor>("ui", "key-distributor");
  if (distributor)
    distributor->deregister_key_observer(this);
}

//--------------------------------------------------------------------------
// Handle event
void UIKeyControl::handle_key(int /*code*/)
{
  send(Dataflow::Value());
}

//--------------------------------------------------------------------------
// Get key code
string UIKeyControl::get_code() const
{
  for (const auto& p: key_name_to_code)
  {
    if (p.second == code)
      return p.first;
  }
  return {};
}

//--------------------------------------------------------------------------
// Set key code
void UIKeyControl::set_code(const string& c)
{
  if (c.empty())
    return;
  const auto& kit = key_name_to_code.find(c);
  if (kit != key_name_to_code.end())
    code = kit->second;
  else
    code = static_cast<int>(c[0]);
}

//--------------------------------------------------------------------------
// Get when
string UIKeyControl::get_when() const
{
  switch (when)
  {
    case When::pressed:
      return "pressed";
    case When::released:
      return "released";
  }
  return "";
}

//--------------------------------------------------------------------------
// Set when
void UIKeyControl::set_when(const string& w)
{
  when = (w == "released" ? When::released : When::pressed);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "key",
  "Key Input",
  "Generic Key Input",
  "ui",
  {
    { "code", { "Key code", Value::Type::text,
                { &UIKeyControl::get_code, &UIKeyControl::set_code }, true } },
    { "when", { "Event trigger - pressed | released", Value::Type::text,
                { &UIKeyControl::get_when, &UIKeyControl::set_when },
          {"pressed", "released"}, true } }
  },
  { { "trigger", { "Key trigger", "trigger", Value::Type::trigger }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(UIKeyControl, module)
