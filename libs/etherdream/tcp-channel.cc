//==========================================================================
// ViGraph Ether Dream protocol library: tcp-channel.cc
//
// TCP Data channel
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-etherdream.h"
#include "ot-log.h"

static const int receive_buffer_size = 1024;

namespace ViGraph { namespace EtherDream {

// Send data
void TCPDataChannel::send(const vector<uint8_t>& data)
{
  try
  {
    if (socket) socket->write(data.data(), data.size());
  }
  catch (Net::SocketError se)
  {
    Log::Error log;
    log << "Ether Dream socket write failed: " << se << endl;
  }
}

// Receive data
size_t TCPDataChannel::receive(vector<uint8_t>& data)
{
  try
  {
    uint8_t buffer[receive_buffer_size];
    ssize_t len = socket?socket->read(buffer, receive_buffer_size):0;
    data.insert(data.end(), buffer, buffer+len);
    return len;
  }
  catch (Net::SocketError se)
  {
    Log::Error log;
    log << "Ether Dream socket read failed: " << se << endl;
    return 0;
  }
}


}} // namespaces
