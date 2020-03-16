//==========================================================================
// ViGraph dataflow module: mqtt-out/mqtt-out.cc
//
// Interface to MQTT IoT protocol, for outputs
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module.h"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#include "mosquittopp.h"

namespace {

const auto default_sample_rate = 1;
const int default_mqtt_port{1883};

using namespace mosqpp;

//==========================================================================
// MQTTOut element
class MQTTOut: public SimpleElement, private mosquittopp
{
private:
  Number last_value{0};

  bool loop_started{false};
  string connected_host;
  int connected_port{0};

  // Element virtuals
  void setup(const SetupContext& context) override;
  void tick(const TickData& td) override;

  // mosquittopp virtuals
  void on_connect(int rc) override;

  // Clone
  MQTTOut *create_clone() const override
  {
    return new MQTTOut{module};
  }

  void shutdown();

public:
  using SimpleElement::SimpleElement;

  // Settings
  Setting<Number> sample_rate{default_sample_rate};
  Setting<string> topic;
  Setting<string> host{"localhost"};
  Setting<Integer> port{default_mqtt_port};

  // Inputs
  Input<Number> input;

  ~MQTTOut() { shutdown(); }
};

//--------------------------------------------------------------------------
// Setup
void MQTTOut::setup(const SetupContext& context)
{
  SimpleElement::setup(context);
  input.set_sample_rate(sample_rate);

  Log::Streams log;
  log.summary << "Creating MQTT output on topic " << topic << endl;

  // Only (re-)connect if changed
  if (host.get() != connected_host || port.get() != connected_port)
  {
    if (!connected_host.empty())
    {
      log.detail << "MQTT: Disconnecting old client\n";
      mosquittopp::disconnect();
    }

    log.detail << "MQTT: Connecting to " << host << " port " << port << endl;
    auto rc = mosquittopp::connect(host.get().c_str(), port);
    if (rc == MOSQ_ERR_SUCCESS)
    {
      log.detail << "MQTT connected OK\n";
      connected_host = host;
      connected_port = port;
    }
    else
    {
      log.error << "Failed to connect to MQTT on " << host << ":" << port
                << ": " << strerror(errno);
      return;
    }
  }

  if (!loop_started)
  {
    mosquittopp::loop_start();
    loop_started = true;
  }
}

//--------------------------------------------------------------------------
// Connection result
void MQTTOut::on_connect(int rc)
{
  Log::Detail log;
  log << "MQTT: connection result: " << rc << endl;
}

//--------------------------------------------------------------------------
// Tick
void MQTTOut::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(sample_rate);
  sample_iterate(td, nsamples, {}, tie(input), {},
                 [&](Number input)
  {
    if (input != last_value)
    {
      string payload{Text::ftos(input)};
      mosquittopp::publish(0, topic.get().c_str(),
                           payload.size(), payload.data());  // No QoS
      input = last_value;
    }
  });
}

//--------------------------------------------------------------------------
// Shut down
void MQTTOut::shutdown()
{
  Log::Summary log;
  log << "Shutting down MQTT output\n";
  if (!connected_host.empty()) mosquittopp::disconnect();
  if (loop_started) mosquittopp::loop_stop();
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "mqtt-out",
  "MQTT Output",
  "iot",
  {
    { "sample-rate", &MQTTOut::sample_rate },
    { "topic",   &MQTTOut::topic },
    { "host",    &MQTTOut::host  },
    { "port",    &MQTTOut::port  }
  },
  {
    { "input",   &MQTTOut::input }
  },
  {}
};
#pragma clang diagnostic pop

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(MQTTOut, module)
