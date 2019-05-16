//==========================================================================
// ViGraph dataflow machines: value.cc
//
// Value struct implementation
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-dataflow.h"

namespace ViGraph { namespace Dataflow {

//------------------------------------------------------------------------
// Get a string representation of a value type
string Value::type_str(Value::Type t)
{
  switch (t)
  {
    default:
    case Value::Type::invalid: return "INVALID!";
    case Value::Type::trigger: return "trigger";
    case Value::Type::number:  return "number";
    case Value::Type::text:    return "text";
    case Value::Type::boolean: return "boolean";
    case Value::Type::choice:  return "choice";
    case Value::Type::file:    return "file";
    case Value::Type::other:   return "other";
    case Value::Type::any:     return "any";
  }
}

//------------------------------------------------------------------------
// Construct from a JSON value
Value::Value(const JSON::Value& json)
{
  switch (json.type)
  {
    case JSON::Value::NUMBER:
      type = Type::number;
      d = json.f;
      break;

    case JSON::Value::INTEGER:
      type = Type::number;
      d = json.n;
      break;

    case JSON::Value::STRING:
      type = Type::text;
      s = json.s;
      break;

    case JSON::Value::TRUE_:
      type = Type::number;
      d = 1.0;
      break;

    case JSON::Value::FALSE_:
      type = Type::number;
      d = 0.0;
      break;

    case JSON::Value::ARRAY:
    case JSON::Value::OBJECT:
      type = Type::other;
      j = json;
      break;

    default:
      type = Type::invalid;
  }
}



}} // namespaces
