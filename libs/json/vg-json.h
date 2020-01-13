//==========================================================================
// ViGraph JSON conversion: vg-json.h
//
// JSON <-> dataflow functions
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VG_JSON_H
#define __VG_JSON_H

#include "ot-json.h"
#include "vg-dataflow.h"

namespace ViGraph { namespace JSON {

// Make our lives easier without polluting anyone else
using namespace std;
using namespace ObTools;
using namespace ViGraph;
using ObTools::JSON::Value;

//--------------------------------------------------------------------------
// Get metadata for module
Value get_module_metadata(const Dataflow::Module& module);

//--------------------------------------------------------------------------
// Get JSON
void get(const Dataflow::Engine& engine, Value& json,
         const Dataflow::Path& path, bool recursive,
         bool show_transient_values);

//--------------------------------------------------------------------------
// Set from JSON
void set(Dataflow::Engine& engine, const Value& json,
         const Dataflow::Path& path);

//--------------------------------------------------------------------------
// Delete by path
void del(Dataflow::Engine& engine, const Dataflow::Path& path);

//==========================================================================
}} //namespaces
#endif // !__VG_JSON_H
