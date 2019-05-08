//==========================================================================
// ViGraph engine server: main.cc
//
// Main file for ViGraph engine server
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "engine.h"

namespace {
const auto server_name    = "ViGraph dataflow engine daemon";
const auto server_version = VERSION;
const auto default_config_file_name = "engine.cfg.xml";

#if defined(PLATFORM_WINDOWS)
const auto default_licence_file_name = "licence.xml";
#else
const auto default_licence = "/etc/vigraph/licence.xml";
#endif

#ifdef DEBUG
const auto default_config_file = default_config_file_name;
#else
const auto default_config_file = "/etc/vigraph/" + default_config_file_name;
#endif
const auto config_file_root = "engine";
const auto default_log_file = "/var/log/vigraph/engine.log";
const auto pid_file         = "/var/run/vg-engine.pid";
}

using namespace std;
using namespace ObTools;
using namespace ViGraph::Engine;

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
#if defined(PLATFORM_WINDOWS)
  winsock_initialise();
  wchar_t p[MAX_PATH];
  GetModuleFileNameW(nullptr, p, MAX_PATH);
  const auto path = File::Path(Text::UTF8::encode(&p[0]));
  const auto licence_file = path.dirname() + "\\" + default_licence_file_name;
  const auto config_file = path.dirname() + "\\" + default_config_file_name;
  Server server(licence_file);
  Daemon::Shell shell(server, server_name, server_version,
                      config_file, config_file_root,
                      default_log_file, pid_file);
#else
  Server server(default_licence);
  Daemon::Shell shell(server, server_name, server_version,
                      default_config_file, config_file_root,
                      default_log_file, pid_file);
#endif
  return shell.start(argc, argv);
}
