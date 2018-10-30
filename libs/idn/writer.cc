//==========================================================================
// ViGraph IDN stream library: writer.cc
//
// IDN message writer
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-idn.h"

namespace ViGraph { namespace IDN {

//-----------------------------------------------------------------------
// Write a single IDN Stream Channel Message
// Throws runtime_error if it fails
void Writer::write(Message& message)
{
  try
  {
    output.write_nbo_16(message.length());
    uint8_t cnl = 0x80 | (message.cclf?0x40:0) | message.channel_id;
    output.write_byte(cnl);
    output.write_byte(static_cast<uint8_t>(message.chunk_type));
    output.write_nbo_32(message.timestamp);

    if (message.has_channel_configuration())
    {
      output.write_byte((message.config.tags.size()+1)/2);// round up to 32bit

      uint8_t cfl = 0;
      if (message.config.routing) cfl |= 0x01;
      if (message.config.close) cfl |= 0x02;
      cfl |= (message.config.sdm & 3) << 4;
      output.write_byte(cfl);
      output.write_byte(message.config.service_id);
      output.write_byte(static_cast<uint8_t>(message.config.service_mode));
      for(const auto& it: message.config.tags)
        output.write_nbo_16(it.to_word());
      // Re-pad to 32-bits with a void tag
      if (message.config.tags.size() & 1)
        output.write_nbo_16(0);
    }

    if (!message.data.empty())
    {
      // Write data header only if not a sequel
      if (!message.is_sequel())
      {
        output.write_byte(message.scm << 4 | (message.once?1:0));
        output.write_nbo_24(message.duration);
      }
      output.write(message.data);
    }
  }
  catch (Channel::Error ce)
  {
    throw runtime_error("IDN stream error: "+ce.text);
  }
}

//-----------------------------------------------------------------------
// Write an IDN-Hello header
// Throws runtime_error if it fails
void Writer::write(HelloHeader& hello)
{
  try
  {
    output.write_byte(static_cast<uint8_t>(hello.command));
    output.write_byte(hello.flags);
    output.write_nbo_16(hello.sequence);
  }
  catch (Channel::Error ce)
  {
    throw runtime_error("IDN hello header error: "+ce.text);
  }
}


}} // namespaces
