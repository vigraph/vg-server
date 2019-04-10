//==========================================================================
// ViGraph dataflow modules: module.h
//
// Headers needed for module build
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VIGRAPH_MODULE_H
#define __VIGRAPH_MODULE_H

#include "ot-log.h"
#include "vg-dataflow.h"

using namespace std;
using namespace ObTools;
using namespace ViGraph;
using namespace ViGraph::Dataflow;

// Macro to define init function with logger and registration for elements
#define VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(_class, _module)                   \
Registry::NewFactory<_class> _new_factory;                                    \
extern "C" bool vg_init(Log::Channel& logger, Dataflow::Engine& engine)       \
{                                                                             \
  Log::logger.connect(new Log::ReferencedChannel{logger});                    \
  Log::Streams log;                                                           \
  log.summary << "  Module: " << _module.id << endl;                          \
  engine.element_registry.add(_module, _new_factory);                         \
  return true;                                                                \
}

#endif // !__VIGRAPH_MODULE_H
