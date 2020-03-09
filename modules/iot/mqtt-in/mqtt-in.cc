//==========================================================================
// ViGraph dataflow module: mqtt-in/mqtt-in.cc
//
// Interface to MQTT IoT protocol, for inputs
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module.h"
#include "mosquittopp.h"

namespace {

const int default_mqtt_port{1883};

using namespace mosqpp;

//==========================================================================
// MQTTIn element
class MQTTIn: public SimpleElement, private mosquittopp
{
private:
  Number last_value{0};

  bool loop_started{false};
  string connected_host;
  int connected_port{0};
  string subscribed_topic;

  // Element virtuals
  void setup(const SetupContext& context) override;
  void tick(const TickData& td) override;

  // mosquittopp virtuals
  void on_connect(int rc) override;
  void on_subscribe(int, int, const int *) override;
  void on_message(const struct mosquitto_message *msg) override;

  // Clone
  MQTTIn *create_clone() const override
  {
    return new MQTTIn{module};
  }

  void shutdown();

public:
  using SimpleElement::SimpleElement;

  // Settings
  Setting<string> topic;
  Setting<string> host{"localhost"};
  Setting<Integer> port{default_mqtt_port};

  // Outputs
  Output<Number> output;

  ~MQTTIn() { shutdown(); }
};

//--------------------------------------------------------------------------
// Setup
void MQTTIn::setup(const SetupContext& context)
{
  SimpleElement::setup(context);

  Log::Streams log;
  log.summary << "Creating MQTT input on topic " << topic << endl;

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

  // New topic?
  if (topic != subscribed_topic)
  {
    // Unsubscribe old
    if (!subscribed_topic.empty())
    {
      log.detail << "MQTT: Unsubscribing from " << subscribed_topic << endl;
      mosquittopp::unsubscribe(0, subscribed_topic.c_str());
    }

    // Subscribe
    if (!topic.get().empty())
    {
      log.detail << "MQTT: Subscribing to " << topic << endl;
      mosquittopp::subscribe(0, topic.get().c_str());
      subscribed_topic = topic;
    }
  }
}

//--------------------------------------------------------------------------
// Connection result
void MQTTIn::on_connect(int rc)
{
  Log::Detail log;
  log << "MQTT: connection result: " << rc << endl;
}

//--------------------------------------------------------------------------
// Subscription result
void MQTTIn::on_subscribe(int, int, const int *)
{
  Log::Detail log;
  log << "MQTT: subscribed\n";
}

//--------------------------------------------------------------------------
// Receive a message
void MQTTIn::on_message(const struct mosquitto_message *msg)
{
  string payload((char *)msg->payload, msg->payloadlen);
  Log::Detail log;
  log << "MQTT: message received: " << "[" << msg->topic << "] = '"
      << payload << "'\n";
  last_value = Text::stof(payload);
}

//--------------------------------------------------------------------------
// Tick
void MQTTIn::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, {}, tie(output),
                 [&](Number& output)
  {
    output = last_value;
  });
}

//--------------------------------------------------------------------------
// Shut down
void MQTTIn::shutdown()
{
  Log::Summary log;
  log << "Shutting down MQTT input\n";
  if (!subscribed_topic.empty())
    mosquittopp::unsubscribe(0, subscribed_topic.c_str());
  if (!connected_host.empty()) mosquittopp::disconnect();
  if (loop_started) mosquittopp::loop_stop();
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "mqtt-in",
  "MQTT Input",
  "iot",
  {
    { "topic",   &MQTTIn::topic },
    { "host",    &MQTTIn::host  },
    { "port",    &MQTTIn::port  }
  },
  {},
  {
    { "output", &MQTTIn::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(MQTTIn, module)
