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
void Packet::write(Channel::Writer& writer) const
{
  writer.write("Art-Net", 8); // Including 0 terminator
  writer.write_le_16((uint16_t)opcode);  // Note LE!
  writer.write_nbo_16(protocol_revision);
}

//-----------------------------------------------------------------------
// Read base from a channel
void Packet::read(Channel::Reader& reader)
{
  try
  {
    string header;
    reader.read(header, 8);
    if (header != string("Art-Net")+(char)0) return;

    opcode = (OpCode)reader.read_le_16();
    reader.read_nbo_16();  // Protocol revision, ignored
  }
  catch (Channel::Error e)
  {
    opcode = op_invalid;
  }
}

//-----------------------------------------------------------------------
// Write ArtDMX to a channel
void DMXPacket::write(Channel::Writer& writer) const
{
  Packet::write(writer);
  writer.write_byte(sequence);
  writer.write_byte(0);  // Physical - not used
  writer.write_le_16(port_address);   // Note LE!
  writer.write_nbo_16(data.size());
  writer.write(data);
}

//-----------------------------------------------------------------------
// Read ArtDMX from a channel
void DMXPacket::read(Channel::Reader& reader)
{
  Packet::read(reader);

  try
  {
    sequence = reader.read_byte();
    reader.read_byte(); // Network, not used
    port_address = reader.read_le_16();
    auto size = reader.read_nbo_16();
    string s;
    reader.read(s, size);
    data.assign(s.begin(), s.end());
  }
  catch (Channel::Error e)
  {
    opcode = op_invalid;
  }
}

}} // namespaces
