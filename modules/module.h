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

// Macros for module init functions - can be used separately in multi-module
// units
#define VIGRAPH_MODULE_NEW_FACTORY(_class, _module) \
Registry::NewFactory<_class, decltype(_module)> _new_factory##_class{_module};

#define VIGRAPH_MODULE_INIT_START                                             \
extern "C" bool vg_init(Log::Channel& logger, Dataflow::Engine& engine)       \
{                                                                             \
  Log::logger.connect(new Log::ReferencedChannel{logger});                    \
  Log::Streams log;

#define VIGRAPH_MODULE_INIT_REGISTER(_class, _module)                         \
  log.summary << "  Module: " << _module.get_full_type() << endl;             \
  engine.element_registry.add(_module.get_section(), _module.get_id(),        \
                              _new_factory##_class);

#define VIGRAPH_MODULE_INIT_END                                               \
  return true;                                                                \
}


// Single simple macro for single-module units
#define VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(_class, _module)                   \
VIGRAPH_MODULE_NEW_FACTORY(_class, _module)                                   \
VIGRAPH_MODULE_INIT_START                                                     \
VIGRAPH_MODULE_INIT_REGISTER(_class, _module)                                 \
VIGRAPH_MODULE_INIT_END

#endif // !__VIGRAPH_MODULE_H
