//==========================================================================
// ViGraph dataflow module: core/containers/graph/graph.cc
//
// Basic graph container module
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../core-module.h"

namespace {

Dataflow::GraphModule module;

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Graph, module)
