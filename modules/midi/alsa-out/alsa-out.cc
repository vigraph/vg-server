//==========================================================================
// Write data to an ALSA MIDI device
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../midi-module.h"
#include <alsa/asoundlib.h>
#include "vg-midi.h"

namespace {

using namespace ViGraph::Dataflow;

const auto default_device = "default";
const auto sample_rate = 1000.0;

class ALSAOutThread;

//==========================================================================
// ALSAOut
class ALSAOut: public SimpleElement
{
private:
  snd_rawmidi_t *midi_out{nullptr};
  unique_ptr<ALSAOutThread> thread;
  atomic<bool> running{false};
  MT::Queue<MIDIEvent> events;

  friend class ALSAOutThread;
  MT::Mutex zero_time_mutex;
  Time::Duration zero_time;
  void run();

  // Source/Element virtuals
  void setup(const SetupContext& context) override;
  void tick(const TickData& td) override;
  void shutdown();

  // Clone
  ALSAOut *create_clone() const override
  {
    return new ALSAOut{module};
  }

public:
  using SimpleElement::SimpleElement;

  Setting<string> device{default_device};

  Input<MIDIEvents> input;

  ~ALSAOut() { shutdown(); }
};

//==========================================================================
// ALSAOut thread
class ALSAOutThread: public MT::Thread
{
private:
  ALSAOut& out;

  void run() override
  { out.run(); }

public:
  ALSAOutThread(ALSAOut& _out): out{_out} {}
};

//--------------------------------------------------------------------------
// Setup
void ALSAOut::setup(const SetupContext& context)
{
  SimpleElement::setup(context);

  Log::Streams log;
  shutdown();

  input.set_sample_rate(sample_rate);

  log.summary << "Opening MIDI output on ALSA device '" << device << "'\n";
  log.detail << "ALSA library version: " << SND_LIB_VERSION_STR << endl;

  auto status = snd_rawmidi_open(nullptr, &midi_out, device.get().c_str(),
                                 SND_RAWMIDI_SYNC | SND_RAWMIDI_NONBLOCK);
  if (status)
  {
    log.error << "Can't open MIDI device: " << snd_strerror(status) << endl;
    return;
  }

  // Clear anything buffered
  snd_rawmidi_drop(midi_out);

  thread.reset(new ALSAOutThread(*this));
  running = true;
  thread->start();
}

//--------------------------------------------------------------------------
// Run background
void ALSAOut::run()
{
  Log::Streams log;

  auto zt = Time::Duration{};
  while (running)
  {
    if (events.poll())
    {
      if (!zt)
      {
        MT::Lock lock{zero_time_mutex};
        zt = zero_time;
      }
      const auto event = events.wait();
      const auto now = Time::Duration::clock();
      const auto time_until = zt + event.time - now;
      if (time_until > Time::Duration{})
        this_thread::sleep_for(chrono::milliseconds{time_until.milliseconds()});
      vector<uint8_t> data;
      auto writer = ViGraph::MIDI::Writer{data};
      writer.write(event);
      const auto write_start = Time::Duration::clock();
      snd_rawmidi_write(midi_out, &data[0], data.size());
      const auto write_end = Time::Duration::clock();
      if (write_end - write_start > Time::Duration{1})
      {
        Log::Error log;
        log << "WRITE TIME: " << (write_end - write_start).iso() << endl;
      }
    }
    else
    {
      this_thread::sleep_for(chrono::milliseconds{1});
    }
  }
}

//--------------------------------------------------------------------------
// Process some data
void ALSAOut::tick(const TickData& td)
{
  {
    MT::Lock lock{zero_time_mutex};
    if (!zero_time)
      zero_time = Time::Duration::clock() - Time::Duration{td.start};
  }
  const auto sample_rate = input.get_sample_rate();
  const auto nsamples = td.samples_in_tick(sample_rate);
  sample_iterate(td, nsamples, {}, tie(input), {},
                 [&](const MIDIEvents& i)
  {
    for (const auto& e: i)
      events.emplace(e);
  });
}

//--------------------------------------------------------------------------
// Shut down
void ALSAOut::shutdown()
{
  Log::Detail log;
  log << "Shutting down ALSA MIDI output\n";

  running = false;
  if (thread) thread->join();

  if (midi_out) snd_rawmidi_close(midi_out);
  midi_out = nullptr;
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::DynamicModule module
{
  "alsa-out",
  "ALSA MIDI output",
  "midi",
  {
    { "device", &ALSAOut::device },
  },
  {
    { "input", &ALSAOut::input },
  },
  {},
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ALSAOut, module)
