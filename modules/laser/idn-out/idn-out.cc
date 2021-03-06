//==========================================================================
// ViGraph dataflow module: laser/idn-out/idn-out.cc
//
// Filter to output display to an IDN stream (UDP)
//
// Copyright (c) 2017-2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector/vector-module.h"
#include "vg-idn.h"

namespace {

using namespace ViGraph::Dataflow;
const int default_packet_size = 1472;  // Max to avoid fragmentation
const double default_config_interval = 0.1;
const auto default_source_address = "0.0.0.0";
const auto default_frame_rate = 50;

//==========================================================================
// IDNOut filter
class IDNOut: public SimpleElement
{
private:
  Net::EndPoint source, destination;

  // State
  unique_ptr<Net::UDPSocket> socket;
  Time::Stamp last_config_sent;
  uint16_t message_sequence{0};

  // Element virtuals
  void setup(const SetupContext& context) override;
  void tick(const TickData& td) override;

  // Clone
  IDNOut *create_clone() const override
  {
    return new IDNOut{module};
  }

  void shutdown();

  // Internal
  void transmit(const Frame& frame, timestamp_t timestamp, double sample_rate);

public:
  using SimpleElement::SimpleElement;

  // Settings
  Setting<string> host_address;
  Setting<Integer> host_port{IDN::Hello::default_port};
  Setting<Integer> packet_size{default_packet_size};
  Setting<Number> config_interval{default_config_interval};
  Setting<bool> intensity_enabled{false};
  Setting<string> source_address{default_source_address};
  Setting<Integer> frame_rate{default_frame_rate};

  // Input
  Input<Frame> input;

  // Destructor
  ~IDNOut() { shutdown(); }
};

//--------------------------------------------------------------------------
// Setup after config
void IDNOut::setup(const SetupContext& context)
{
  SimpleElement::setup(context);

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

  input.set_sample_rate(frame_rate);
}

//--------------------------------------------------------------------------
// Tick data
void IDNOut::tick(const TickData& td)
{
  const auto sample_rate = frame_rate;
  auto sample_time = td.first_sample_at(sample_rate);
  const auto sample_duration = td.sample_duration(sample_rate);
  const auto nsamples = td.samples_in_tick(sample_rate);
  sample_iterate(td, nsamples, {}, tie(input), {},
                 [&](const Frame& input)
  {
    transmit(input, sample_time, sample_rate);
    sample_time += sample_duration;
  });
}

//--------------------------------------------------------------------------
// Transmit a frame
void IDNOut::transmit(const Frame& frame, timestamp_t timestamp,
                      double sample_rate)
{
  if (!socket) return;

  // Construct message
  IDN::Message message(IDN::Message::ChunkType::laser_frame_samples_entire);

  // Timestamp and duration in microseconds, wrapping every ~4000 sec
  message.timestamp = timestamp * 1000000;
  message.set_data_header(1000000/sample_rate);

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
  if (!frame.points.empty())
  {
    const auto& p = frame.points[0];
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
       point_index<frame.points.size()+(point_index?0:1);)
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
      min(data_space / bytes_per_point, frame.points.size()-point_index);

    if (!point_index)
    {
      // Do we need to fragment?
      if (points_this_message != frame.points.size())
      {
        // Change the chunk type
        message.chunk_type = IDN::Message::ChunkType::laser_frame_samples_first;
      }
    }
    else
    {
      // Subsequent fragment - change CCLF for last fragment
      message.cclf = point_index+points_this_message >= frame.points.size();
    }

    // Add data
    for(size_t i=0; i<points_this_message; i++)
    {
      const auto& p = frame.points[point_index+i];
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

    // Send it
    try
    {
      socket->sendto(packet.data(), size, 0, destination);
    }
    catch (Net::SocketError e)
    {
      Log::Error log;
      log << "IDN transmit socket error: " << e.get_string() << endl;
      return;
    }

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
// Shut down
void IDNOut::shutdown()
{
  Log::Detail log;
  log << "Shutting down IDN transmit server\n";
  socket.reset();
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "idn-out",
  "IDN Output",
  "laser",
  {
    { "packet-size",     &IDNOut::packet_size       },
    { "config-interval", &IDNOut::config_interval   },
    { "intensity",       &IDNOut::intensity_enabled },
    { "address",         &IDNOut::host_address      },
    { "port",            &IDNOut::host_port         },
    { "source-address",  &IDNOut::source_address    },
    { "frame-rate",      &IDNOut::frame_rate        }
  },
  {
    { "input",           &IDNOut::input }
  },
  {}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(IDNOut, module)

