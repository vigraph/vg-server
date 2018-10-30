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
const char *default_config_interval = "0.1";

//==========================================================================
// IDNTransmit filter
class IDNTransmitFilter: public FrameFilter
{
  unsigned int packet_size;
  Time::Duration config_interval;
  Net::EndPoint source, destination;
  unique_ptr<Net::UDPSocket> socket;
  Time::Stamp last_config_sent;
  uint16_t message_sequence{0};
  bool intensity_enabled{false};

  // Source/Element virtuals
  void configure(const XML::Element& config) override;
  void accept(FramePtr frame) override;
  void shutdown() override;

public:
  IDNTransmitFilter(const Dataflow::Module *module, const XML::Element& config):
    Element(module, config), FrameFilter(module, config) {}
};

//--------------------------------------------------------------------------
// Configure from XML
void IDNTransmitFilter::configure(const XML::Element& config)
{
  XML::ConstXPathProcessor xpath(config);
  Log::Streams log;

  string address = xpath["host/@address"];
  int port = config.get_attr_int("port", IDN::Hello::default_port);
  destination = Net::EndPoint(Net::IPAddress(address), port);
  log.summary << "Creating IDN transmitter to " << destination << endl;

  // Read configuration
  packet_size = xpath.get_value_int("packet/@size", default_packet_size);
  log.detail << " - packet size " << packet_size << endl;

  config_interval = Time::Duration(xpath.get_value("configuration/@interval",
                                                   default_config_interval));
  log.detail << " - configuration interval: " << config_interval.seconds()
             << "s\n";

  // Option to add intensity
  intensity_enabled = xpath.get_value_bool("intensity/@enabled");

  source = Net::EndPoint(
        Net::IPAddress(xpath.get_value("source/@address", "0.0.0.0")), 0);
  // Bind to local port
  socket.reset(new Net::UDPSocket(source, true));

  source = socket->local();
  log.detail << "IDN transmitter bound to local address "
             << source << endl;
}

//--------------------------------------------------------------------------
// Process some data
void IDNTransmitFilter::accept(FramePtr frame)
{
  // Construct message
  IDN::Message message(IDN::Message::ChunkType::laser_frame_samples_entire);

  // Timestamp and duration in microseconds, wrapping every ~4000 sec
  message.timestamp = static_cast<uint32_t>(frame->timestamp * 1000000.0);
  message.duration = static_cast<uint32_t>(graph->get_tick_interval().seconds()
                                           * 1000000.0);

  // Add configuration periodically
  Time::Stamp now = Time::Stamp::now();
  if (now-last_config_sent >= config_interval)
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

  // Loop, possibly fragmenting
  for (size_t point_index=0; point_index<frame->points.size();)
  {
    // Check size before adding the rest of the data, to see if we need
    // to fragment
    size_t size = message.length();
    IDN::HelloHeader hello(IDN::HelloHeader::Command::message,
                           message_sequence++);
    size += hello.length();

    // If even this won't fit, nothing we can do
    if (size > packet_size)
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

    point_index += points_this_message;

    // Change chunk type and clear for next fragment (if any) - doing it
    // here ensures it is taken into account in the header size() next time
    message.chunk_type = IDN::Message::ChunkType::laser_frame_samples_sequel;
    message.data.clear();

    // Increment timestamp for next fragment
    message.timestamp++;
  }

  // Send it down as well, so these can be chained
  send(frame);
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
    { "packet.size",
      { { "UDP packet size in bytes", "1472" },
          Value::Type::number, "packet/@size" } },
    { "config.interval",
      { { "Interval between configuration packets in sec", "0.1" },
          Value::Type::number, "configuration/@interval" } },
    { "intensity.enabled",
      { "Whether to send overall intensity values",
          Value::Type::boolean, "intensity/@enabled" } },
    { "host.address",
      { "Destination host (IP or hostname)",
          Value::Type::text, "host/@address" } },
    { "host.port",
      { { "Destination port", "7255" },
          Value::Type::number, "host/@port" } },
    { "source.address",
      { { "Source address to bind to", "0.0.0.0"},
          Value::Type::text, "source/@address" } }
  },
  { "VectorFrame" }, // inputs
  { "VectorFrame" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(IDNTransmitFilter, module)

