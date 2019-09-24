//==========================================================================
// ViGraph Art-Net protocol library: packet.cc
//
// Packet writers
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-artnet.h"

namespace ViGraph { namespace ArtNet {

//-----------------------------------------------------------------------
// Write base to a channel
void Packet::write(Channel::Writer& writer)
{
  writer.write("Art-Net", 8); // Including 0 terminator
  writer.write_le_16((uint16_t)opcode);  // Note LE!
  writer.write_nbo_16(protocol_revision);
}

//-----------------------------------------------------------------------
// Write ArtDMX to a channel
void DMXPacket::write(Channel::Writer& writer)
{
  Packet::write(writer);
  writer.write_byte(sequence);
  writer.write_byte(0);  // Physical - not used
  writer.write_le_16(port_address);   // Note LE!
  writer.write_nbo_16(data.size());
  writer.write(data);
}

}} // namespaces
