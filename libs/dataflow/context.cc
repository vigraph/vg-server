//==========================================================================
// ViGraph dataflow machines: context.cc
//
// Setup Context implementation
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-dataflow.h"

namespace ViGraph { namespace Dataflow {

File::Path SetupContext::get_file_path(const string& path) const
{
  const auto full_path = resource_dir.resolve(path);
  const auto abs_path = full_path.realpath();
  const auto d = resource_dir.realpath().str();
  const auto p = abs_path.str();
  if (p.size() < d.size() || d != p.substr(0, d.size()))
    throw(runtime_error{"Invalid file path: " + path});
  return abs_path;
}

}} // namespaces
