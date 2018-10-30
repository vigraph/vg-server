//==========================================================================
// ViGraph dataflow module: ui/managers/key-distributor/key-distributor.cc
//
// Accepts raw key codes and distributes triggers to individual targets
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include "ot-web.h"

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

//--------------------------------------------------------------------------
class KeyDistributor: public Dataflow::Manager
{
  struct Target
  {
    string id;
    string property;
    Dataflow::Element *element{nullptr};
    Target() {}
    Target(const string& _id, const string& _prop=""):
      id(_id), property(_prop) {}
  };
  map<int, Target> key_targets;  // By key code (negative for release)

  // Element virtuals
  void connect() override;
  void set_property(const string& property, const SetParams& sp) override;

public:
  // Construct
  KeyDistributor(const Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML:
//   <key-distributor>
//     <key code="xxx" target="yyy"/>
//     <key code="xxx" when="released" target="yyy"/>
//     ...
//   </key-distributor>
KeyDistributor::KeyDistributor(const Module *module,
                               const XML::Element& config):
  Element(module, config), Manager(module, config)
{
  for(const auto& it: config.get_children("key"))
  {
    const auto& ke = *it;
    const auto& code_str = ke["code"];
    if (code_str.empty()) throw runtime_error("No key code given in "+id);
    int code = 0;
    const auto& kit = key_name_to_code.find(code_str);
    if (kit != key_name_to_code.end())
      code = kit->second;
    else
      code = static_cast<int>(code_str[0]);
    if (ke["when"] == "released") code = -code;

    const auto& target_id = ke["target"];
    const auto& property = ke.get_attr("property", "trigger");
    key_targets[code] = Target(target_id, property);
  }
}

//--------------------------------------------------------------------------
// Connect to other elements in the graph
void KeyDistributor::connect()
{
  for(auto& kt: key_targets)
  {
    auto& target = kt.second;
    target.element = graph->get_element(target.id);
    if (!target.element)
      throw runtime_error("No such target element "+target.id
                          +" referred to by "+id);
    target.element->notify_target_of(this);
  }
}

//--------------------------------------------------------------------------
// Set a control property
void KeyDistributor::set_property(const string& property,
                                  const SetParams& sp)
{
  if (property == "key")
  {
    // Look up the key code
    int code = static_cast<int>(sp.v.d);
    const auto& it = key_targets.find(code);
    if (it != key_targets.end())
    {
      const auto& target = it->second;
      if (target.element)
      {
        SetParams trigger_sp{Dataflow::Value()};
        target.element->set_property(target.property, trigger_sp);
      }
    }
  }
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "key-distributor",
  "Key Distributor",
  "Accepts key presses and distributes them to multiple trigger targets",
  "ui",
  {
    { "", { "Key map", Value::Type::text } }, // How to describe subelements!!
    { "key", { "Key code", Value::Type::number, true } }
  },
  { { "", { "Key trigger", "trigger", Value::Type::trigger }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(KeyDistributor, module)
