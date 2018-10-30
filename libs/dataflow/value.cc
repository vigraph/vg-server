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
    case Value::Type::any:     return "any";
  }
}



}} // namespaces
