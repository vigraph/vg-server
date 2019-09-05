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
  json.set("id", module.get_id());
  json.set("name", module.get_name());
  json.set("section", module.get_section());

  // Settings
  if (module.has_settings())
  {
    auto& settingsj = json.put("settings", Value{Value::ARRAY});
    module.for_each_setting([&settingsj](const string& id,
                               const Dataflow::Module::SettingMember& setting)
    {
      auto& sj = settingsj.add(Value{Value::OBJECT});
      sj.set("id", id);
      sj.set("type", setting.get_type());
    });
  }

  // Inputs
  if (module.has_inputs())
  {
    auto& inputsj = json.put("inputs", Value{Value::ARRAY});
    module.for_each_input([&inputsj](const string& id,
                               const Dataflow::Module::InputMember& input)
    {
      auto& ij = inputsj.add(Value{Value::OBJECT});
      ij.set("id", id);
      ij.set("type", input.get_type());
    });
  }

  // Outputs
  if (module.has_outputs())
  {
    auto& outputsj = json.put("outputs", Value{Value::ARRAY});
    module.for_each_output([&outputsj](const string& id,
                               const Dataflow::Module::OutputMember& output)
    {
      auto& oj = outputsj.add(Value{Value::OBJECT});
      oj.set("id", id);
      oj.set("type", output.get_type());
    });
  }

  return json;
}

}} // namespaces
