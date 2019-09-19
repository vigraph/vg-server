//==========================================================================
// ViClone dataflow module: core/containers/clone/clone.cc
//
// Clone container module
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../core-module.h"

namespace {

Dataflow::CloneModule module;

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Clone, module)
