//==========================================================================
// ViGraph Ether Dream protocol library: interface.cc
//
// Abstract protocol interface
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-etherdream.h"

namespace ViGraph { namespace EtherDream {

// Start the interface
void Interface::start()
{
  vector<uint8_t> data;
  data.push_back(0x3f);  // '?' = ping
  send_data(data);
}

}} // namespaces
