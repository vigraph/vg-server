//==========================================================================
// ViGraph Ether Dream protocol library: tcp-interface.cc
//
// TCP Interface implementations
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-etherdream.h"
#include "ot-log.h"

namespace ViGraph { namespace EtherDream {

// Start the interface
bool TCPInterface::start()
{
  Log::Streams log;
  log.summary << "Starting Ether Dream TCP interface to " << device << endl;

  client.reset(new Net::TCPClient(device, timeout));
  if (!*client)
  {
    log.error << "Can't connect to " << device << endl;
    return false;
  }

  tcp_channel.set_socket(client.get());
  return Interface::start();
}

}} // namespaces
