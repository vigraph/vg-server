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
  json.set("id", module.id);
  json.set("name", module.name);
  json.set("section", module.section);

  // Settings
  if (!module.settings.empty())
  {
    auto& settingsj = json.put("settings", Value{Value::ARRAY});
    for(const auto sit: module.settings)
    {
      const auto& id = sit.first;
      const auto& setting = sit.second;
      auto& sj = settingsj.add(Value{Value::OBJECT});
      sj.set("id", id);
      sj.set("type", setting.type);
    }
  }

  // Inputs
  if (!module.inputs.empty())
  {
    auto& inputsj = json.put("inputs", Value{Value::ARRAY});
    for(const auto iit: module.inputs)
    {
      const auto& id = iit.first;
      const auto& input = iit.second;
      auto& ij = inputsj.add(Value{Value::OBJECT});
      ij.set("id", id);
      ij.set("type", input.type);
    }
  }

  // Outputs
  if (!module.outputs.empty())
  {
    auto& outputsj = json.put("inputs", Value{Value::ARRAY});
    for(const auto oit: module.outputs)
    {
      const auto& id = oit.first;
      const auto& output = oit.second;
      auto& oj = outputsj.add(Value{Value::OBJECT});
      oj.set("id", id);
      oj.set("type", output.type);
    }
  }

  return json;
}

}} // namespaces
