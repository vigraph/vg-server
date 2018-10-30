//==========================================================================
// ViGraph IDN stream library: reader.cc
//
// IDN message reader
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-idn.h"

namespace ViGraph { namespace IDN {

//-----------------------------------------------------------------------
// Read a single IDN Stream Channel Message
// Throws runtime_error if it fails
void Reader::read(Message& message)
{
  try
  {
    uint16_t length = input.read_nbo_16();
    if (length<8) throw runtime_error("IDN message too short");

    uint8_t cnl = input.read_byte();
    if (!(cnl & 0x80)) throw runtime_error("IDN message CNL top bit not set");
    message.cclf = (cnl & 0x40) != 0;
    message.channel_id = cnl & 0x3f;

    uint8_t chunk_type = input.read_byte();
    Message::ChunkType& ct = message.chunk_type;
    switch (chunk_type)
    {
      case 0x00: ct = Message::ChunkType::void_;                      break;
      case 0x01: ct = Message::ChunkType::laser_wave_samples;         break;
      case 0x02: ct = Message::ChunkType::laser_frame_samples_entire; break;
      case 0x03: ct = Message::ChunkType::laser_frame_samples_first;  break;
      case 0x10: ct = Message::ChunkType::octet_segment;              break;
      case 0x11: ct = Message::ChunkType::octet_string;               break;
      case 0x18: ct = Message::ChunkType::dimmer_levels;              break;
      case 0xc0: ct = Message::ChunkType::laser_frame_samples_sequel; break;
      default:   throw runtime_error("Unrecognised chunk type in IDN");
    }

    message.timestamp = input.read_nbo_32();
    length -= 8;

    // Optional channel configuration
    if (message.has_channel_configuration())
    {
      uint8_t scwc = input.read_byte();
      uint8_t cfl = input.read_byte();
      if (cfl & 1) message.set_routing();
      if (cfl & 2) message.set_close();
      message.config.sdm = (cfl >> 4) & 3;
      message.config.service_id = input.read_byte();

      uint8_t service_mode = input.read_byte();
      Message::Config::ServiceMode& sm = message.config.service_mode;
      switch (service_mode)
      {
        case 0x00: sm = Message::Config::ServiceMode::void_;              break;
        case 0x01: sm = Message::Config::ServiceMode::graphic_continuous; break;
        case 0x02: sm = Message::Config::ServiceMode::graphic_discrete;   break;
        case 0x03: sm = Message::Config::ServiceMode::effects_continuous; break;
        case 0x04: sm = Message::Config::ServiceMode::effects_discrete;   break;
        case 0x05: sm = Message::Config::ServiceMode::dmx512_continuous;  break;
        case 0x06: sm = Message::Config::ServiceMode::dmx512_discrete;    break;
        default:   throw runtime_error("Unrecognised service mode in IDN");
      }

      length -= 4;

      for(auto i=0; i<scwc*2; i++)  // 2 tags per 32-bit word
      {
        uint16_t tag_word = input.read_nbo_16();
        Tag tag(tag_word);

        // Check for Void
        if (!tag.category && !tag.subcategory)
        {
          // Skip extra in parameter if requested
          if (tag.parameter)
          {
            input.skip(2*tag.parameter);
            length -= 2*tag.parameter;
            i+=tag.parameter;
          }
          // but don't add to message
        }
        else message.add_tag(tag);
        length -= 2;
      }
    }

    // If there is any left, there must be data
    if (length >= 4)
    {
      // Only read a header if not a sequel message
      if (!message.is_sequel())
      {
        uint8_t flags = input.read_byte();
        message.scm = (flags >> 4) & 3;
        message.once = (flags & 1) != 0;
        message.duration = input.read_nbo_24();
        length -= 4;
      }

      // Whatever length is left is data
      input.read_to_eof(message.data, length);
    }
  }
  catch (Channel::Error ce)
  {
    throw runtime_error("IDN stream error: "+ce.text);
  }
}

//-----------------------------------------------------------------------
// Read an IDN Hello header
// Throws runtime_error if it fails
void Reader::read(HelloHeader& hello)
{
  try
  {
    uint8_t command = input.read_byte();
    HelloHeader::Command& cmd = hello.command;
    switch (command)
    {
      case 0x00: cmd = HelloHeader::Command::void_;               break;
      case 0x08: cmd = HelloHeader::Command::ping_request;        break;
      case 0x09: cmd = HelloHeader::Command::ping_reply;          break;
      case 0x10: cmd = HelloHeader::Command::scan_request;        break;
      case 0x11: cmd = HelloHeader::Command::scan_reply;          break;
      case 0x40: cmd = HelloHeader::Command::message;             break;
      default:   throw runtime_error("Unrecognised command in IDN Hello");
    }

    hello.flags = input.read_byte();
    hello.sequence = input.read_nbo_16();
  }
  catch (Channel::Error ce)
  {
    throw runtime_error("IDN Hello error: "+ce.text);
  }
}


}} // namespaces
