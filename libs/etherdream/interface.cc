//==========================================================================
// ViGraph Ether Dream protocol library: interface.cc
//
// Abstract protocol interface
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-etherdream.h"

namespace ViGraph { namespace EtherDream {

// Command bytes
enum Command
{
  prepare           = 0x70,   // 'p'
  begin_playback    = 0x62,   // 'b'
  queue_rate_change = 0x71,   // 'q'
  write_data        = 0x64,   // 'd'
  stop              = 0x73,   // 's'
  e_stop            = 0,
  clear_e_stop      = 0x63,   // 'c'
  ping              = 0x3f    // '?'
};

// Start the interface
void Interface::start()
{
  vector<uint8_t> data;
  data.push_back(Command::ping);
  send_data(data);
}

// Prepare stream
void Interface::prepare()
{
  vector<uint8_t> data{ Command::prepare };
  send_data(data);
}

// Begin playback
void Interface::begin_playback(uint32_t point_rate)
{
  vector<uint8_t> data(7);
  Channel::BlockWriter bw(data);

  bw.write_byte(Command::begin_playback);
  bw.write_le_16(0);   // 'low water mark' (not implemented their side)
  bw.write_le_32(point_rate);

  send_data(data);
}

// Queue rate change
void Interface::queue_rate_change(uint32_t point_rate)
{
  vector<uint8_t> data(5);
  Channel::BlockWriter bw(data);

  bw.write_byte(Command::queue_rate_change);
  bw.write_le_32(point_rate);

  send_data(data);
}

// Send data
void Interface::send(vector<Point>& points)
{
  vector<uint8_t> data(3+18*points.size());
  Channel::BlockWriter bw(data);

  bw.write_byte(Command::write_data);
  bw.write_le_16(points.size());

  for(const auto& p: points)
  {
    bw.write_le_16(0);  // control
    bw.write_le_16(static_cast<int16_t>(65535*p.x));
    bw.write_le_16(static_cast<int16_t>(65535*p.y));
    bw.write_le_16(static_cast<uint16_t>(65535*p.c.r));
    bw.write_le_16(static_cast<uint16_t>(65535*p.c.g));
    bw.write_le_16(static_cast<uint16_t>(65535*p.c.b));
    bw.write_le_16(0);  // i ?intensity?
    bw.write_le_16(0);  // u1
    bw.write_le_16(0);  // u2
  }

  send_data(data);
}

// Stop
void Interface::stop_playback()
{
  vector<uint8_t> data{ Command::stop };
  send_data(data);
}

// Emergency stop
void Interface::emergency_stop()
{
  vector<uint8_t> data{ Command::e_stop };
  send_data(data);
}

// Clear emergency stop
void Interface::clear_emergency_stop()
{
  vector<uint8_t> data{ Command::clear_e_stop };
  send_data(data);
}

// Receive data
void Interface::receive_data(const vector<uint8_t>& data)
{
  receive_buffer.insert(receive_buffer.end(), data.begin(), data.end());
  if (last_status.read(receive_buffer))
    receive_buffer.erase(receive_buffer.begin(), receive_buffer.begin()+20);
}

}} // namespaces
