//==========================================================================
// ViGraph vector graphics: vg-etherdream.h
//
// Support for Etherdream Laser DAC
// Protocol definitions from: https://www.ether-dream.com/protocol.html
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VG_ETHERDREAM_H
#define __VG_ETHERDREAM_H

#include "ot-chan.h"
#include "ot-net.h"
#include "vg-geometry.h"

namespace ViGraph { namespace EtherDream {

// Make our lives easier without polluting anyone else
using namespace std;
using namespace ViGraph;
using namespace ViGraph::Geometry;
using namespace ObTools;

const auto default_port = 7765;

struct Status
{
  uint8_t protocol{0};           // ? Not defined in doc

  enum class LightEngineState
  {
    ready    = 0,
    warmup   = 1,
    cooldown = 2,
    e_stop   = 3
  };
  LightEngineState light_engine_state{LightEngineState::ready};

  enum class PlaybackState
  {
    idle     = 0,
    prepared = 1,
    playing  = 2
  };
  PlaybackState playback_state{PlaybackState::idle};

  enum class Source
  {
    network  = 0,
    sd_card  = 1,
    internal = 2
  };
  Source source{Source::network};

  enum LightEngineFlags
  {
    e_stop_network          = 1,
    e_stop_external         = 2,
    e_stop_active           = 4,
    e_stop_over_temperature = 8,
    over_temperature_active = 16,
    e_stop_no_link          = 32
  };

  uint16_t light_engine_flags{0};

  enum PlaybackFlags
  {
    shutter_open = 1,
    underflow    = 2,
    e_stop       = 4
  };
  uint16_t playback_flags{0};

  uint16_t source_flags{0};   // ? Not defined in doc
  uint16_t buffer_fullness{0};
  uint32_t point_rate{0};
  uint32_t point_count{0};

  // Read and remove from raw data
  bool read(vector<uint8_t>& data);

  // Dump to channel
  void dump(ostream& out) const;
};

//==========================================================================
// Data channel abstract interface
class DataChannel
{
 public:
  // Send data
  virtual void send(const vector<uint8_t>& data) = 0;

  // Receive data - blocking, result is 0 if channel fails
  virtual size_t receive(vector<uint8_t>&) { return 0; }

  virtual ~DataChannel() {}
};

//==========================================================================
// EtherDream command sender
// Separated out for one-way message testing
class CommandSender
{
 private:
  DataChannel& channel;

 public:
  CommandSender(DataChannel& c): channel(c) {}

  // Prepare stream
  void prepare();

  // Begin playback
  void begin_playback(uint32_t point_rate);

  // Queue rate change
  void queue_rate_change(uint32_t point_rate);

  // Send data, with option to send rate change on first point
  void send(const vector<Point>& points, bool change_rate = false);

  // Stop
  void stop_playback();

  // Emergency stop
  void emergency_stop();

  // Clear emergency stop
  void clear_emergency_stop();

  // Send ping
  void ping();
};

//==========================================================================
// Ether Dream Interface
// Handles request/response using the given data channel
class Interface
{
  DataChannel& channel;
  vector<uint8_t> receive_buffer;
  Status last_status;
  CommandSender commands;

  // Internal
  bool get_response();

 public:
   Interface(DataChannel& c):
     channel(c), commands(c) {}

  // Start the interface
  // Returns true if initial 'response' received and ping accepted
  virtual bool start();

  // Ensure the interface is ready to receive data
  // Returns whether it is ready
  bool get_ready();

  // Send point data to the interface
  // duration of this frame in seconds
  // Returns whether data sent successfully
  bool send(const vector<Point>& points, double duration);

  // Get last status (for testing)
  const Status& get_last_status() { return last_status; }

  // Get estimate of buffer availability
  size_t get_buffer_points_available();
};

//==========================================================================
// TCP Data channel
class TCPDataChannel: public DataChannel
{
  Net::TCPSocket *socket{0};

  void send(const vector<uint8_t>& data) override;
  size_t receive(vector<uint8_t>&) override;

 public:
  void set_socket(Net::TCPSocket *s) { socket = s; }
};

//==========================================================================
// TCP Interface
class TCPInterface: public Interface
{
  Net::EndPoint device;
  int timeout;
  TCPDataChannel tcp_channel;
  unique_ptr<Net::TCPClient> client;

 public:
  static const int default_timeout = 5;

  TCPInterface(Net::EndPoint _dev,
               int _timeout = default_timeout):
    Interface(tcp_channel),
    device(_dev), timeout(_timeout)
    {}

  // Start the interface
  bool start() override;
};

//==========================================================================
}} //namespaces
#endif // !__VG_ETHERDREAM_H
