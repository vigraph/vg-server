//==========================================================================
// ViGraph dataflow module: object/nexus-client/nexus-client.cc
//
// Nexus client object data source
//
// Copyright (c) 2021 Paul Clark.  All rights reserved
//==========================================================================

#include "ot-ssl-openssl.h"
#include "../object-module.h"
#include "ot-web.h"

namespace {

const auto user_agent("ViGraph Nexus client/0.1");
const auto fetch_timeout{5};

//==========================================================================
// Nexus client
class NexusClient: public SimpleElement
{
private:
  // Source/Element virtuals
  void setup(const SetupContext& context) override;
  void tick(const TickData& td) override;

  // Clone
  NexusClient *create_clone() const override
  {
    return new NexusClient{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Settings
  Setting<string> url;

  // Output
  Output<Data> output;
};

//--------------------------------------------------------------------------
// Setup
void NexusClient::setup(const SetupContext& context)
{
  SimpleElement::setup(context);

  Log::Streams log;
  Web::URL wurl(url);
  SSL_OpenSSL::Context ssl;
  Web::HTTPClient client(wurl, &ssl, user_agent, fetch_timeout, fetch_timeout);
  string body;
  log.detail << "Fetching data from " << wurl << endl;
}

//--------------------------------------------------------------------------
// Generate a frame
void NexusClient::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, {}, tie(output),
                 [&](Data& output)
  {
    output.json = JSON::Value(JSON::Value::OBJECT);
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "nexus-client",
  "Nexus client",
  "object",
  {
    { "url",      &NexusClient::url     }
  },
  {},
  {
    { "output",   &NexusClient::output  }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(NexusClient, module)
