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
#include <thread>
#include <sstream>

namespace {

const auto user_agent("ViGraph Nexus client/0.1");

//==========================================================================
// Nexus client
class NexusClient: public SimpleElement
{
private:
  unique_ptr<Web::HTTPClient> http;
  unique_ptr<Web::WebSocketServer> ws;
  MT::Mutex last_json_mutex;
  JSON::Value last_json{JSON::Value::OBJECT};
  atomic<bool> stop_fetch_thread{false};
  unique_ptr<thread> fetch_thread;
  bool is_active{false};

  // Source/Element virtuals
  void setup(const SetupContext& context) override;
  void tick(const TickData& td) override;

  // Internal
  bool reconnect();

  // Clone
  NexusClient *create_clone() const override
  {
    return new NexusClient{module};
  }

  // Destructor
  ~NexusClient();

public:
  using SimpleElement::SimpleElement;

  // Settings
  Setting<string> url;
  Setting<string> resource;

  // Output
  Output<Data> output;
  Output<Number> active;
};

//--------------------------------------------------------------------------
// (Re)connect the WebSocket
bool NexusClient::reconnect()
{
  Log::Streams log;
  log.detail << "Connecting to Nexus at " << url << endl;

  Web::URL wurl(url);
  SSL_OpenSSL::Context ssl;
  ws.reset();
  http.reset(new Web::HTTPClient(wurl, &ssl, user_agent));

  Net::TCPStream *stream;
  auto rc = http->open_websocket(Web::URL("/"), stream);
  if (rc != 101)
  {
    log.error << "Can't open Websocket at " << url << endl;
    http.reset();
    return false;
  }

  http->enable_keepalive();
  ws.reset(new Web::WebSocketServer(*stream));

  // Subscribe
  JSON::Value subscribe(JSON::Value::OBJECT);
  subscribe.set("type", "subscribe");
  if (!resource.get().empty()) subscribe.set("resource", resource.get());
  ws->write(subscribe.str());

  return true;
}

//--------------------------------------------------------------------------
// Setup
void NexusClient::setup(const SetupContext& context)
{
  SimpleElement::setup(context);

  // Set up websocket
  // Don't worry if it fails, the thread will pick this up and retry
  reconnect();

  // Start the thread to read messages
  fetch_thread.reset(new thread([this]
  {
    while (!stop_fetch_thread)
    {
      string msg;
      if (!ws || !ws->read(msg))
      {
        if (stop_fetch_thread) break;

        // Wait then reconnect
        this_thread::sleep_for(1s);
        reconnect();
        continue;
      }

      istringstream iss(msg);
      JSON::Parser parser(iss);
      JSON::Value json;
      try
      {
        json = parser.read_value();
      }
      catch (JSON::Exception e)
      {
        Log::Error log;
        log << "Bad JSON: " << e.error << endl;
        continue;
      }

      const auto& mtype = json["type"].as_str();

      if (mtype == "control")
      {
        const auto& values = json["values"];
        if (values.type == JSON::Value::OBJECT)
        {
          MT::Lock lock(last_json_mutex);
          last_json = values;
          is_active = true;
        }
      }
      else if (mtype == "idle")
      {
        is_active = false;
      }
    }
  }));
}

//--------------------------------------------------------------------------
// Generate a frame
void NexusClient::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(max(output.get_sample_rate(),
                                               active.get_sample_rate()));
  sample_iterate(td, nsamples, {}, {}, tie(output, active),
                 [&](Data& output, Number& active)
  {
    MT::Lock lock(last_json_mutex);
    output.json = last_json;
    active = is_active;
  });
}

//--------------------------------------------------------------------------
// Destructor - stop thread and close WebSocket
NexusClient::~NexusClient()
{
  if (!!ws) ws->close();
  stop_fetch_thread = true;
  if (!!fetch_thread) fetch_thread->join();
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "nexus-client",
  "Nexus client",
  "object",
  {
    { "url",      &NexusClient::url      },
    { "resource", &NexusClient::resource }
  },
  {},
  {
    { "output",   &NexusClient::output  },
    { "active",   &NexusClient::active  }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(NexusClient, module)
