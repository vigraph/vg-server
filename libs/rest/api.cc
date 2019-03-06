//==========================================================================
// ViGraph REST API library: api.cc
//
// REST API implementation
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-rest.h"
#include "ot-log.h"

namespace ViGraph { namespace REST {

// -------------------------------------------------------------------------
// Handle a graph request
API::Result API::handle_graph_request(const string& verb,
                                      const vector<string>& bits,
                                      const JSON::Value& /*data*/)
{
  if (bits.empty())
  {
    // Top level graph
    if (verb != "GET")
      return API::Result(API::Result::Code::method_not_allowed);

    API::Result result(API::Result::Code::ok);
    result.value = engine.get_graph().get_json();
    return result;
  }
  else
  {
    // !!!
    return API::Result(API::Result::Code::not_found);
  }
}

// -------------------------------------------------------------------------
// Handle a meta request
API::Result API::handle_meta_request(const string& /*verb*/,
                                     const vector<string>& /*bits*/,
                                     const JSON::Value& /*data*/)
{
  return API::Result(API::Result::Code::not_found);
}

// -------------------------------------------------------------------------
// Handle a request
// Verb is GET, PUT, POST, DELETE
// Path is top-level path - e.g. "/graph/foo"
API::Result API::handle_request(const string& verb,
                                const string& path,
                                const JSON::Value& data)
{
  Log::Detail log;
  log << "REST API: " << verb << " " << path << endl;
  if (!!data)
  {
    OBTOOLS_LOG_IF_DEBUG(Log::Debug dlog; dlog << data;)
  }

  // Remove leading /
  auto rpath = path;
  if (!rpath.empty() && rpath[0] == '/') rpath.erase(rpath.begin());
  if (rpath.empty()) return API::Result(API::Result::Code::not_found);

  // Split on /
  auto bits = Text::split(rpath, '/');

  // Remove first
  const auto& prefix = bits[0];
  bits.erase(bits.begin());

  // Check overall prefixes
  if (prefix == "graph")
    return handle_graph_request(verb, bits, data);
  else if (prefix == "meta")
    return handle_meta_request(verb, bits, data);
  else
    return API::Result(API::Result::Code::not_found);
}



}} // namespaces
