//==========================================================================
// ViGraph JSON: module.cc
//
// JSON module functions
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-json.h"

namespace ViGraph { namespace JSON {

Value get_module_metadata(const Dataflow::Module& module)
{
  auto json = Value{Value::OBJECT};
  json.set("name", module.get_name());

  // Settings
  if (module.has_settings())
  {
    auto& settingsj = json.put("settings", Value{Value::OBJECT});
    module.for_each_setting([&settingsj](const string& id,
                               const Dataflow::Module::SettingMember& setting)
    {
      auto& sj = settingsj.put(id, Value{Value::OBJECT});
      sj.set("type", setting.get_type());
    });
  }

  // Inputs
  if (module.has_inputs())
  {
    auto& inputsj = json.put("inputs", Value{Value::OBJECT});
    module.for_each_input([&inputsj](const string& id,
                               const Dataflow::Module::InputMember& input)
    {
      auto& ij = inputsj.put(id, Value{Value::OBJECT});
      ij.set("type", input.get_type());
    });
  }

  // Outputs
  if (module.has_outputs())
  {
    auto& outputsj = json.put("outputs", Value{Value::OBJECT});
    module.for_each_output([&outputsj](const string& id,
                               const Dataflow::Module::OutputMember& output)
    {
      auto& oj = outputsj.put(id, Value{Value::OBJECT});
      oj.set("type", output.get_type());
    });
  }

  return json;
}

}} // namespaces
