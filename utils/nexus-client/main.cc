//==========================================================================
// ViGraph Nexus test client: main.cc
//
// Utility to test the Nexus communications / queue system
//
// Copyright (c) 2021 Paul Clark.  All rights reserved
//==========================================================================

#include "ot-web.h"
#include "ot-json.h"

using namespace std;
using namespace ObTools;

namespace
{
  const int default_server_port = 33480;
}

//--------------------------------------------------------------------------
// Usage
void usage(const char *argv0)
{
  cout << "ViGraph Nexus test client version " << VERSION << "\n\n";
  cout << "Usage:\n";
  cout << "  " << argv0 << " [options]\n\n";
  cout << "Options:\n";
  cout << "  -q --quiet                   No version header\n";
  cout << "  -v --verbose                 More verbose\n";
  cout << "  -h --server-host <hostname>  Connect to given host (localhost)\n";
  cout << "  -p --port <port>             Connect to given port ("
       << default_server_port << ")\n";
  cout << " -js --jwt-secret <secret>     Set secret for JWT authentication (none)\n";
  cout << endl;
}

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  auto log_level = Log::Level::summary;
  bool quiet = false;
  string server_host{"localhost"};
  int server_port{default_server_port};
  string jwt_secret;
  for(int i=1; i<argc; i++)
  {
    string arg(argv[i]);
    if (arg=="-q" || arg=="--quiet")
      quiet = true;
    else if (arg == "-v" || arg == "--verbose")
      log_level = static_cast<Log::Level>(static_cast<int>(log_level)+1);
    else if ((arg=="-h" || arg=="--server-host") && i<argc-1)
      server_host = argv[++i];
    else if ((arg=="-p" || arg=="--server-port") && i<argc-1)
      server_port = Text::stoi(argv[++i]);
    else if ((arg=="-js" || arg=="--jwt-secret") && i<argc-1)
      jwt_secret = argv[++i];
    else
    {
      cerr << "Unknown option: " << arg << endl;
      usage(argv[0]);
      return 2;
    }
  }

  Net::EndPoint server(server_host, server_port);

  // Set up logging
  auto chan_out = new Log::StreamChannel{&cerr};
  auto level_out = new Log::LevelFilter{chan_out, log_level};
  Log::logger.connect(level_out);

  if (!quiet)
  {
    cout << "ViGraph Nexus test client version " << VERSION << endl;
    cout << "Connecting WebSocket to " << server << endl;
  }

  // Create a JWT for it
  JSON::Value payload(JSON::Value::OBJECT);
  // !!! Fill some stuff in?
  Web::JWT jwt(payload);
  string auth_header;
  if (!jwt_secret.empty())
  {
    jwt.sign(jwt_secret);
    auth_header = "Bearer "+jwt.str();
  }

  Web::HTTPClient http_client(server);
  Net::TCPStream *stream;
  auto rc = http_client.open_websocket(Web::URL("/"), stream, auth_header);
  if (rc != 101)
  {
    cerr << "Error: " << rc << endl;
    return 4;
  }

  Web::WebSocketServer ws(*stream);

  // Send a join
  ws.write("{ type: \"join\" }");

  for(;;)
  {
    string msg;
    if (ws.read(msg))
      cout << "WS: " << msg << endl;
    else
      break;
  }

  return 0;
}




