//==========================================================================
// ViGraph engine server: main.cc
//
// Main file for ViGraph engine server
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "engine.h"
#include "vg-service.h"

using namespace ViGraph::Engine;

namespace {
const auto server_name    = "ViGraph dataflow engine daemon";
const auto server_version = VERSION;

const auto default_licence = "/etc/vigraph/licence.xml";
#ifdef DEBUG
const auto default_config_file = "engine.cfg.xml";
#else
const auto default_config_file = "/etc/vigraph/engine.cfg.xml";
#endif

const auto config_file_root = "engine";
const auto default_log_file = "/var/log/vigraph/engine.log";
const auto pid_file         = "/var/run/vg-engine.pid";
}

int main(int argc, char **argv)
{
  Server server(default_licence);
  Daemon::Shell shell(server, server_name, server_version,
                      default_config_file, config_file_root,
                      default_log_file, pid_file);
  return shell.start(argc, argv);
}
