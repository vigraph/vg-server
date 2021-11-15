//==========================================================================
// ViGraph Nexus test client: main.cc
//
// Utility to test the Nexus communications / queue system
//
// Copyright (c) 2021 Paul Clark.  All rights reserved
//==========================================================================

#include "ot-web.h"
#include "ot-json.h"
#include "ot-log.h"
#include "ot-file.h"
#include "ot-script.h"
#include "ot-cache.h"
#include "ot-misc.h"

using namespace std;
using namespace ObTools;

//==========================================================================
// Globals

// Cache of active clients

struct Client
{
  unique_ptr<Web::HTTPClient> http;
  unique_ptr<Web::WebSocketServer> ws;
};

typedef Cache::BasicPointerCache<int, Client> client_cache_t;
client_cache_t clients;  // By session ID
MT::Mutex client_id_mutex;
int client_id = 0;

//==========================================================================
// Open action
class OpenAction: public Script::SingleAction
{
public:
  using Script::SingleAction::SingleAction;

  //------------------------------------------------------------------------
  // Run action
  bool run(Script::Context& con)
  {
    auto server = Net::EndPoint(xml["server"]);
    auto client = new Client;
    client->http.reset(new Web::HTTPClient(server));
    Net::TCPStream *stream;
    auto rc = client->http->open_websocket(Web::URL("/"), stream);
    if (rc != 101)
    {
      Log::Error log;
      log << "Can't open Websocket to " << server << endl;
      return false;
    }
    client->ws.reset(new Web::WebSocketServer(*stream));

    int id;

    {
      MT::Lock lock(client_id_mutex);
      id = ++client_id;
    }

    clients.add(id, client);
    Log::Summary log;
    log << "Client " << id << " opened\n";

    // Add it to context for other operations
    con.vars.add("client", id);

    return true;
  }
};

Init::NewFactory<Script::Action, OpenAction,
                 Script::Action::CP> open_factory;

//==========================================================================
// Join action
class JoinAction: public Script::SingleAction
{
public:
  using Script::SingleAction::SingleAction;

  //------------------------------------------------------------------------
  // Run action
  bool run(Script::Context& con)
  {
    int client_id = con.vars.get_int("client");

    Log::Streams log;
    log.detail << "Joining queue on client " << client_id << endl;

    auto client = clients.lookup(client_id);
    if (client && client->ws)
    {
      client->ws->write("{ type: \"join\" }");
      return true;
    }
    else
    {
      log.error << "No nexus client for ID '" << client_id << "'\n";
      return false;
    }
  }
};

Init::NewFactory<Script::Action, JoinAction,
                 Script::Action::CP> join_factory;

//==========================================================================
// Wait action - wait for a single message
class WaitAction: public Script::SingleAction
{
public:
  using Script::SingleAction::SingleAction;

  //------------------------------------------------------------------------
  // Run action
  bool run(Script::Context& con)
  {
    int client_id = con.vars.get_int("client");
    auto client = clients.lookup(client_id);
    if (client && client->ws)
    {
      string msg;
      if (client->ws->read(msg))
      {
        Log::Detail log;
        log << client_id << ": " << msg << endl;
        return true;
      }
      else return false;
    }
    else return false;
  }
};

Init::NewFactory<Script::Action, WaitAction,
                 Script::Action::CP> wait_factory;

//==========================================================================
// Close action
class CloseAction: public Script::SingleAction
{
public:
  using Script::SingleAction::SingleAction;

  //------------------------------------------------------------------------
  // Run action
  bool run(Script::Context& con)
  {
    Log::Streams log;
    auto client_id = con.vars.get_int("client");
    log.detail << "Closing client " << client_id << endl;

    auto client = clients.lookup(client_id);
    if (client && client->ws) client->ws->close();

    clients.remove(client_id);
    return true;
  }
};

Init::NewFactory<Script::Action, CloseAction,
                 Script::Action::CP> close_factory;

//==========================================================================
// Nexus Test language
class TestLanguage: public Script::BaseLanguage
{
public:
  //------------------------------------------------------------------------
  // Constructor
  TestLanguage(): Script::BaseLanguage()
  {
    register_action("open", open_factory);
    register_action("join", join_factory);
    register_action("wait", wait_factory);
    register_action("close", close_factory);
  }
};

//==========================================================================
// Command line

//--------------------------------------------------------------------------
// Usage
void usage(const char *argv0)
{
  cout << "ViGraph Nexus test client version " << VERSION << "\n\n";
  cout << "Usage:\n";
  cout << "  " << argv0 << " [options] [<configuration file>]\n\n";
  cout << "Options:\n";
  cout << "  -q --quiet                   No version header\n";
  cout << "  -v --verbose                 More verbose\n";
  cout << endl;
}

//--------------------------------------------------------------------------
// Run a script
// Note must only be run once per process
void run(const XML::Element& script_xml)
{
  Log::Streams log;

  // Create our language
  TestLanguage language;

  // Create the script
  Script::Script script(language, script_xml);

  // Run script
  log.summary << "Starting script\n";
  script.run();
  log.summary << "Script finished\n";
}

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  int i = 1;
  for (; i < argc; ++i)
  {
    string arg = argv[i];
    if (arg[0] != '-') break;

    if (arg == "-?" || arg == "--help")
    {
      usage(argv[0]);
      return 0;
    }
    else cerr << "Unknown option " << arg << " ignored" << endl;
  }

  // Grab config filename if specified
  XML::Configuration config;
  string conf_file;
  if (i < argc)
    conf_file = argv[i];
  else
    conf_file = "nexus-client.cfg.xml";

  // Read config
  config.add_file(conf_file);
  if (!config.read("nexus-client")) return 2;

  // Set up logging
  auto chan_out = new Log::StreamChannel{&cout};
  const auto log_level = static_cast<Log::Level>(
    config.get_value_int("log/@level", static_cast<int>(Log::Level::summary)));
  const auto time_format = config["log/@timestamp"];
  Log::logger.connect_full(chan_out, log_level, time_format);
  Log::Streams log;

  log.summary << "ViGraph Nexus test client version " << VERSION << endl;
  log.summary << "Running configuration " << conf_file << endl;

  // Read the script
  XML::Element& root = config.get_root();
  const XML::Element& script_xml = root.get_child("script");

  // Run it
  run(script_xml);
  return 0;
}




