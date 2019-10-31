//==========================================================================
// ViGraph vector graphics: vg-artnet.h
//
// Support for Art-Net lighting control format
// https://art-net.org.uk/resources/art-net-specification/
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VG_ARTNET_H
#define __VG_ARTNET_H

#include "ot-chan.h"

namespace ViGraph { namespace ArtNet {

// Make our lives easier without polluting anyone else
using namespace std;
using namespace ViGraph;
using namespace ObTools;

// Constants
static const auto max_universes{32768u};
static const auto udp_port{0x1936u};
static const auto protocol_revision{14u};

// OpCodes
enum OpCode
{
  op_poll                = 0x2000,
  op_poll_reply          = 0x2100,
  op_diag_data           = 0x2300,
  op_command             = 0x2400,
  op_dmx                 = 0x5000,
  op_nzs                 = 0x5100,
  op_sync                = 0x5200,
  op_address             = 0x6000,
  op_input               = 0x7000,
  op_tod_request         = 0x8000,
  op_tod_data            = 0x8100,
  op_tod_control         = 0x8200,
  op_rdm                 = 0x8300,
  op_rdm_sub             = 0x8400,
  op_media               = 0x9000,
  op_media_patch         = 0x9100,
  op_media_control       = 0x9200,
  op_media_control_reply = 0x9300,
  op_time_code           = 0x9700,
  op_time_sync           = 0x9800,
  op_trigger             = 0x9900,
  op_directory           = 0x9a00,
  op_directory_reply     = 0x9b00,
  op_video_setup         = 0xa010,
  op_video_palette       = 0xa020,
  op_video_data          = 0xa040,
  op_mac_master          = 0xf000,
  op_mac_slave           = 0xf100,
  op_firmware_master     = 0xf200,
  op_firmware_reply      = 0xf300,
  op_file_tn_master      = 0xf400,
  op_file_fn_master      = 0xf500,
  op_file_fn_reply       = 0xf600,
  op_ip_prog             = 0xf800,
  op_ip_prog_reply       = 0xf900
};

// NodeReport codes
enum NodeReportCode
{
  rc_debug         = 0x0000,
  rc_power_ok      = 0x0001,
  rc_power_fail    = 0x0002,
  rc_socket_wr_1   = 0x0003,
  rc_parse_fail    = 0x0004,
  rc_udp_fail      = 0x0005,
  rc_sh_name_ok    = 0x0006,
  rc_lo_name_ok    = 0x0007,
  rc_dmx_error     = 0x0008,
  rc_dmx_udp_full  = 0x0009,
  rc_dmx_rx_full   = 0x000a,
  rc_switch_err    = 0x000b,
  rc_config_err    = 0x000c,
  rc_dmx_short     = 0x000d,
  rc_firmware_fail = 0x000e,
  rc_user_fail     = 0x000f,
  rc_factory_res   = 0x0010
};

// Style codes
enum Style
{
  st_node       = 0x00,
  st_controller = 0x01,
  st_media      = 0x02,
  st_route      = 0x03,
  st_backup     = 0x04,
  st_config     = 0x05,
  st_visual     = 0x06
};

//==========================================================================
// Art-Net Packet base
struct Packet
{
  OpCode opcode;

  // Constructor
  Packet(OpCode _opcode): opcode(_opcode) {}

  // Write to a channel
  virtual void write(Channel::Writer& writer);
};

//==========================================================================
// Art-Net DMX Packet base
struct DMXPacket: public Packet
{
  uint8_t sequence;
  uint16_t port_address;  // Net(7), Subnet(4), Universe(4)
  vector<uint8_t> data;

  // Constructor
  DMXPacket(uint8_t _sequence, uint16_t _port_address):
    Packet(OpCode::op_dmx),
    sequence(_sequence), port_address(_port_address) {}

  // Write to a channel
  void write(Channel::Writer& writer);

  // Get packet length
  size_t length() { return 18u + data.size(); }
};

//==========================================================================
}} //namespaces
#endif // !__VG_ARTNET_H
