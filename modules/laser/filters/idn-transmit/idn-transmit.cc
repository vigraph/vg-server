//==========================================================================
// ViGraph dataflow module: laser/filters/idn-transmit/idn-transmit.cc
//
// Filter to output display to an IDN stream (UDP)
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../vector/vector-module.h"
#include "vg-idn.h"

namespace {

using namespace ViGraph::Dataflow;
const int default_packet_size = 1472;  // Max to avoid fragmentation
const double default_config_interval = 0.1;
const auto default_source_address = "0.0.0.0";

//==========================================================================
// IDNTransmit filter
class IDNTransmitFilter: public FrameSink
{
public:
  // Config
  string host_address;
  int host_port = IDN::Hello::default_port;
  int packet_size = default_packet_size;
  double config_interval = default_config_interval;
  bool intensity_enabled = false;
  string source_address = default_source_address;

private:
  Net::EndPoint source, destination;

  // State
  unique_ptr<Net::UDPSocket> socket;
  Time::Stamp last_config_sent;
  uint16_t message_sequence{0};
  bool frame_seen{false};

  // Source/Element virtuals
  void setup() override;
  void accept(FramePtr frame) override;
  void post_tick(const TickData&) override;
  void shutdown() override;

  // Internal
  void transmit(FramePtr frame);

public:
  using FrameSink::FrameSink;
};

//--------------------------------------------------------------------------
// Configure from XML
void IDNTransmitFilter::setup()
{
  Log::Streams log;

  destination = Net::EndPoint(Net::IPAddress(host_address), host_port);
  log.summary << "Creating IDN transmitter to " << destination << endl;

  log.detail << " - packet size " << packet_size << endl;
  log.detail << " - configuration interval: " << config_interval << "s\n";

  source = Net::EndPoint(Net::IPAddress(source_address), 0);
  // Bind to local port
  socket.reset(new Net::UDPSocket(source, true));

  source = socket->local();
  log.detail << "IDN transmitter bound to local address " << source << endl;
}

//--------------------------------------------------------------------------
// Process some data
void IDNTransmitFilter::accept(FramePtr frame)
{
  transmit(frame);
  frame_seen = true;
}

//--------------------------------------------------------------------------
// Transmit a frame
void IDNTransmitFilter::transmit(FramePtr frame)
{
  // Construct message
  IDN::Message message(IDN::Message::ChunkType::laser_frame_samples_entire);

  // Timestamp and duration in microseconds, wrapping every ~4000 sec
  message.timestamp = static_cast<uint32_t>(frame->timestamp * 1000000.0);
  auto& engine = graph->get_engine();
  auto duration = static_cast<uint32_t>(engine.get_tick_interval().seconds()
                                           * 1000000.0);
  message.set_data_header(duration);

  // Add configuration periodically
  Time::Stamp now = Time::Stamp::now();
  if ((now-last_config_sent).seconds() >= config_interval)
  {
    message.add_configuration(
                        IDN::Message::Config::ServiceMode::graphic_discrete);
    message.set_routing();

    // Add standard tags
    message.add_tag(IDN::Tags::x);
    message.add_tag(IDN::Tags::prec16);
    message.add_tag(IDN::Tags::y);
    message.add_tag(IDN::Tags::prec16);
    message.add_tag(IDN::Tags::red);
    message.add_tag(IDN::Tags::green);
    message.add_tag(IDN::Tags::blue);
    if (intensity_enabled) message.add_tag(IDN::Tags::intensity);

    last_config_sent = now;
  }

  // If not blank, add a blank point the same as the start
  if (!frame->points.empty())
  {
    const auto& p = frame->points[0];
    message.add_data16(static_cast<uint16_t>(p.x*65535));
    message.add_data16(static_cast<uint16_t>(p.y*65535));
    message.add_data(0);  // r,g,b blank
    message.add_data(0);
    message.add_data(0);
    if (intensity_enabled) message.add_data(0);
  }

  // Loop, possibly fragmenting - first time, allow empty packets if
  // config not already sent
  for (size_t point_index=0;
       point_index<frame->points.size()+(point_index?0:1);)
  {
    // Check size before adding the rest of the data, to see if we need
    // to fragment
    size_t size = message.length();
    IDN::HelloHeader hello(IDN::HelloHeader::Command::message,
                           message_sequence++);
    size += hello.length();

    // If even this won't fit, nothing we can do
    if (size > static_cast<unsigned>(packet_size))
      throw runtime_error("Packet size too small for headers");

    size_t data_space = packet_size-size;
    size_t bytes_per_point = intensity_enabled?8:7;
    size_t points_this_message =
      min(data_space / bytes_per_point, frame->points.size()-point_index);

    if (!point_index)
    {
      // Do we need to fragment?
      if (points_this_message != frame->points.size())
      {
        // Change the chunk type
        message.chunk_type = IDN::Message::ChunkType::laser_frame_samples_first;
      }
    }
    else
    {
      // Subsequent fragment - change CCLF for last fragment
      message.cclf = point_index+points_this_message >= frame->points.size();
    }

    // Add data
    for(size_t i=0; i<points_this_message; i++)
    {
      const auto& p = frame->points[point_index+i];
      message.add_data16(static_cast<uint16_t>(p.x*65535));
      message.add_data16(static_cast<uint16_t>(p.y*65535));
      message.add_data(static_cast<uint8_t>(p.c.r*255));
      message.add_data(static_cast<uint8_t>(p.c.g*255));
      message.add_data(static_cast<uint8_t>(p.c.b*255));
      if (intensity_enabled)
        message.add_data(static_cast<uint8_t>(p.c.get_intensity()*255));
    }

    // Recheck the length
    size = message.length() + hello.length();

    // Create a packet
    vector<unsigned char> packet;
    packet.resize(size);
    Channel::BlockWriter bw(packet.data(), size);
    IDN::Writer writer(bw);
    writer.write(hello);
    writer.write(message);

    // Send it !!! exceptions
    socket->sendto(packet.data(), size, 0, destination);

    // If nothing sent this time, that's it
    if (!points_this_message) break;
    point_index += points_this_message;

    // Change chunk type and clear for next fragment (if any) - doing it
    // here ensures it is taken into account in the header size() next time
    message.chunk_type = IDN::Message::ChunkType::laser_frame_samples_sequel;
    message.data.clear();

    // Increment timestamp for next fragment
    message.timestamp++;
  }
}

//--------------------------------------------------------------------------
// Post-tick flush
void IDNTransmitFilter::post_tick(const TickData& td)
{
  // Send an empty frame if none seen since last tick
  if (!frame_seen)
  {
    FramePtr frame(new Frame(td.t));
    transmit(frame);
  }
  frame_seen = false;
}

//--------------------------------------------------------------------------
// Shut down
void IDNTransmitFilter::shutdown()
{
  Log::Detail log;
  log << "Shutting down IDNTransmit server\n";
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "idn-transmit",
  "IDN Transmitter",
  "ILDA Digital Network (IDN) UDP transmitter",
  "laser",
  {
    { "packet.size", { "UDP packet size in bytes", Value::Type::number,
                       &IDNTransmitFilter::packet_size, true } },
    { "config.interval", { "Interval between configuration packets in sec",
                           Value::Type::number,
                           &IDNTransmitFilter::config_interval, true } },
    { "intensity.enabled", { "Whether to send overall intensity values",
                             Value::Type::boolean,
                             &IDNTransmitFilter::intensity_enabled, true } },
    { "host.address", { "Destination host (IP or hostname)", Value::Type::text,
                        &IDNTransmitFilter::host_address, false } },
    { "host.port", { "Destination port", Value::Type::number,
                     &IDNTransmitFilter::host_port, false } },
    { "source.address", { "Source address to bind to", Value::Type::text,
                          &IDNTransmitFilter::source_address, false } }
  },
  { "VectorFrame" }, // inputs
  { }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(IDNTransmitFilter, module)

