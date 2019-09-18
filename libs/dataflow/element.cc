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
                      GraphElement& b, const string &in_name)
{
  auto& module = get_module();
  auto o = module.get_output(*this, out_name);
  if (!o)
  {
    Log::Error log;
    log << "Unknown output '" << out_name << "' on element "
        << id << endl;
    return false;
  }
  auto& bmodule = b.get_module();
  auto i = bmodule.get_input(b, in_name);
  if (!i)
  {
    Log::Error log;
    log << "Unknown input '" << out_name << "' on element "
        << b.id << endl;
    return false;
  }
  o->connect({&b, i});
  return true;
}

//--------------------------------------------------------------------------
// Notify that connection has been made to input
void Element::notify_connection(const string& in_name,
                                GraphElement&, const string&)
{
  auto& module = get_module();
  auto i = module.get_input(*this, in_name);
  if (!i)
    return;
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
