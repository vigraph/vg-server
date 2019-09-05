//==========================================================================
// ViGraph dataflow machines: element.cc
//
// Element implementation
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-dataflow.h"

namespace ViGraph { namespace Dataflow {

//--------------------------------------------------------------------------
// Connect an element
bool Element::connect(const string& out_name,
                      Element& b, const string &in_name)
{
  auto& module = get_module();
  auto o = module.get_output(*this, out_name);
  if (!o)
    return false;
  auto& bmodule = b.get_module();
  auto i = bmodule.get_input(b, in_name);
  if (!i)
    return false;
  o->connect({&b, i});
  b.inputs.insert(i);
  return true;
}

//--------------------------------------------------------------------------
// Ready - check for tick readiness
bool Element::ready() const
{
  for (const auto& i: inputs)
    if (!i->ready())
      return false;
  return true;
}

//--------------------------------------------------------------------------
// Reset - prepare for tick
void Element::reset()
{
  for (auto& i: inputs)
    i->reset();
}

//--------------------------------------------------------------------------
// Accept a visitor
void Element::accept(Visitor& visitor)
{
  visitor.visit(*this);
}

}} // namespaces
