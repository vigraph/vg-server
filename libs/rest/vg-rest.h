//==========================================================================
// ViGraph REST API library: vg-rest.h
//
// Interface to REST API
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VG_REST_H
#define __VG_REST_H

#include "vg-dataflow.h"
#include "ot-json.h"

namespace ViGraph { namespace REST {

using namespace std;
using namespace ViGraph::Dataflow;
using namespace ObTools;

// -------------------------------------------------------------------------
// REST API
class API
{
  Engine& engine;

 public:
  struct Result
  {
    enum class Code
    {
      ok,
      not_found,
      method_not_allowed,
      bad_request
    } code;

    string detail;
    JSON::Value value;

    Result(Code _code = Code::ok, const string& _detail="",
           JSON::Value::Type _value_type = JSON::Value::Type::OBJECT):
      code(_code), detail(_detail), value(_value_type) {}
  };

 private:
  // Internals
  Result handle_graph_request(const string& verb,
                              const vector<string>& bits,
                              const JSON::Value& data);

  Result handle_meta_request(const string& verb,
                             const vector<string>& bits,
                             const JSON::Value& data);

 public:
  // Constructor
  API(Engine& _engine): engine(_engine) {}

  // Handle a request
  // Verb is GET, PUT, POST, DELETE
  // Path is top-level path - e.g. "/graph/foo"
  Result handle_request(const string& verb,
                        const string& path,
                        const JSON::Value& data);

  // Destructor
  ~API() {}
};

//==========================================================================
}} //namespaces
#endif // !__VG_REST_H
