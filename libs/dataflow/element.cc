//==========================================================================
// ViGraph dataflow machines: element.cc
//
// Element implementation
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-dataflow.h"

namespace ViGraph { namespace Dataflow {

//------------------------------------------------------------------------
// Get type of a control property - uses module by default
Value::Type Element::get_property_type(const string& property)
{
  if (!module) return Value::Type::invalid;
  const auto pit = module->properties.find(property);
  if (pit == module->properties.end()) return Value::Type::invalid;
  return pit->second.type;
}



}} // namespaces
