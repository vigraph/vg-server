//==========================================================================
// ViGraph dataflow module: load-dscusb/load-dscusb.cc
//
// Sensor interface for Mantracourt DSCUSB load cell interface
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module.h"
#include "ot-serial.h"
#include <cmath>

namespace {

const auto default_device{"/dev/dscusb"};
const auto default_interval{0.1};

class SensorThread;  // forward

//==========================================================================
// Sensor element
class Sensor: public SimpleElement
{
private:
  Serial::TTY tty;

  unique_ptr<SensorThread> thread;
  bool running = false;
  friend class SensorThread;
  void run();

  atomic<double> last_value{0.0};

  // Element virtuals
  void setup(const SetupContext& context) override;
  void tick(const TickData& td) override;

  // Clone
  Sensor *create_clone() const override
  {
    return new Sensor{module};
  }

  void shutdown() override;

public:
  using SimpleElement::SimpleElement;

  // Settings
  Setting<string> device{default_device};
  Setting<Number> interval{default_interval};

  // Outputs
  Output<Number> output;
};

//==========================================================================
// Sensor thread
class SensorThread: public MT::Thread
{
private:
  Sensor& sensor;

  void run() override
  { sensor.run(); }

public:
  SensorThread(Sensor& _sensor): sensor(_sensor)
  { start(); }
};

//--------------------------------------------------------------------------
// Setup
void Sensor::setup(const SetupContext& context)
{
  SimpleElement::setup(context);

  Log::Streams log;
  log.summary << "DSCUSB load sensor connecting to " << device << endl;

  if (!tty.open(device))
  {
    log.error << "Failed to open TTY at " << device << endl;
    return;
  }

  auto params = Serial::Parameters{};
  params.in_speed = 115200;
  params.out_speed = 115200;
  params.char_flags = Serial::CharFlags::ignore_modem_control |
                      Serial::CharFlags::enable_receiver |
                      Serial::CharFlags::char_size_8;
  params.local_flags = Serial::LocalFlags::canonical_mode;
  params.min_chars_for_non_canon_read = 1;

  if (!tty.set_parameters(params))
  {
    log.error << "Failed to set TTY parameters on " << device << endl;
  }

  log.summary << "DSCUSB load sensor connected\n";
  running = true;
  thread.reset(new SensorThread(*this));
}

//--------------------------------------------------------------------------
// Run background
void Sensor::run()
{
  while (running)
  {
    tty.write_line("!001:SYS?");

    string input;
    auto timeout = chrono::milliseconds{100};
    if (tty.get_line(input, timeout) == Serial::TTY::GetLineResult::ok)
      last_value = Text::stof(input);

    this_thread::sleep_for(chrono::duration<double>{interval});
  }
}

//--------------------------------------------------------------------------
// Tick
void Sensor::tick(const TickData& td)
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
void Sensor::shutdown()
{
  Log::Summary log;
  log << "Shutting down DSCUSB load sensor\n";

  running = false;
  if (thread) thread->join();

  if (tty) tty.close();
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "load-dscusb",
  "DSCUSB Load",
  "sensor",
  {
    { "device",   &Sensor::device },
    { "interval", &Sensor::interval }
  },
  {},
  {
    { "output", &Sensor::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Sensor, module)
