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
  auto oit = module.outputs.find(out_name);
  if (oit == module.outputs.end())
    return false;
  auto iit = b.module.inputs.find(in_name);
  if (iit == b.module.inputs.end())
    return false;
  auto i = &(iit->second.get(b));
  auto o = &(oit->second.get(*this));
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
