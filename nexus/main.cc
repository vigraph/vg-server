//==========================================================================
// ViGraph nexus server: main.cc
//
// Main file for ViGraph nexus server
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "nexus.h"

using namespace ViGraph::Nexus;

namespace {
const auto server_name    = "ViGraph dataflow nexus daemon";
const auto server_version = VERSION;

#ifdef DEBUG
const auto default_config_file = "nexus.cfg.xml";
#else
const auto default_config_file = "/etc/vigraph/nexus.cfg.xml";
#endif

const auto config_file_root = "nexus";
const auto default_log_file = "/var/log/vigraph/nexus.log";
const auto pid_file         = "/var/run/vg-nexus.pid";
}

int main(int argc, char **argv)
{
  Crypto::Library library;
  Server server;
  Daemon::Shell shell(server, server_name, server_version,
                      default_config_file, config_file_root,
                      default_log_file, pid_file);
  return shell.start(argc, argv);
}
