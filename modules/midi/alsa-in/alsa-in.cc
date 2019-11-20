//==========================================================================
// ViGraph dataflow module: midi/alsa-in/alsa-in.cc
//
// Read data from an ALSA MIDI device
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../midi-module.h"
#include <alsa/asoundlib.h>
#include "vg-midi.h"

namespace {

using namespace ViGraph::Dataflow;

const auto default_device = "default";

class ALSAInThread;

//==========================================================================
// ALSAIn
class ALSAIn: public SimpleElement
{
private:
  snd_rawmidi_t *midi_in{nullptr};
  unique_ptr<ALSAInThread> thread;
  atomic<bool> running{false};
  ViGraph::MIDI::Reader reader;
  struct MIDIEvent
  {
    Time::Duration t;
    MIDI::Event e;
    MIDIEvent(const Time::Duration& _t, const MIDI::Event& _e):
      t{_t}, e{_e} {}
  };
  MT::Mutex events_mutex;
  queue<MIDIEvent> events;
  Time::Duration event_last_read;
  Time::Duration last_tick_end;

  friend class ALSAInThread;
  void run();

  // Source/Element virtuals
  void setup(const SetupContext& context) override;
  void tick(const TickData& td) override;
  void shutdown() override;

  // Clone
  ALSAIn *create_clone() const override
  {
    return new ALSAIn{module};
  }

public:
  using SimpleElement::SimpleElement;

  Setting<string> device{default_device};

  Output<MIDI::Event> output;
};

//==========================================================================
// ALSAIn thread
class ALSAInThread: public MT::Thread
{
private:
  ALSAIn& in;

  void run() override
  { in.run(); }

public:
  ALSAInThread(ALSAIn& _in): in{_in}
  { start(); }
};

//--------------------------------------------------------------------------
// Setup
void ALSAIn::setup(const SetupContext&)
{
  Log::Streams log;
  shutdown();

  log.summary << "Opening MIDI input on ALSA device '" << device << "'\n";
  log.detail << "ALSA library version: " << SND_LIB_VERSION_STR << endl;

  auto status = snd_rawmidi_open(&midi_in, nullptr, device.get().c_str(),
                                 SND_RAWMIDI_SYNC | SND_RAWMIDI_NONBLOCK);
  if (status)
  {
    log.error << "Can't open MIDI device: " << snd_strerror(status) << endl;
    return;
  }

  // Clear anything buffered
  snd_rawmidi_drop(midi_in);

  thread.reset(new ALSAInThread(*this));
  running = true;
}

//--------------------------------------------------------------------------
// Run background
void ALSAIn::run()
{
  Log::Streams log;

  // Get FDs for poll
  const auto nfds = snd_rawmidi_poll_descriptors_count(midi_in);
  auto fds = vector<struct pollfd>(nfds);
  snd_rawmidi_poll_descriptors(midi_in, fds.data(), nfds);

  // Filter for only input side
  auto ifds = vector<struct pollfd>{};
  for (auto i=0; i<nfds; i++)
  {
    if (fds[i].events & POLLIN)
      ifds.push_back(fds[i]);
  }

  while (running)
  {
    if (poll(ifds.data(), ifds.size(), 100) > 0)
    {
      unsigned char bytes[100];
      auto n = snd_rawmidi_read(midi_in, bytes, sizeof(bytes));
      if (n < 0)
      {
        log.error << "Can't read MIDI input: " << snd_strerror(n) << endl;
        this_thread::sleep_for(chrono::milliseconds{100});
        continue;
      }
      event_last_read = Time::Duration::clock();

      for(auto i=0; i<n; i++)
        reader.add(bytes[i]);

      auto ev = reader.get();
      MT::Lock lock{events_mutex};
      while (ev.type != MIDI::Event::Type::none)
      {
        events.emplace(event_last_read, ev);
        ev = reader.get();
      }
    }
  }
}

//--------------------------------------------------------------------------
// Process some data
void ALSAIn::tick(const TickData& td)
{
  const auto sample_rate = output.get_sample_rate();
  const auto nsamples = td.samples_in_tick(sample_rate);
  const auto tick_duration = Time::Duration{td.duration()};
  const auto sample_duration = Time::Duration{td.sample_duration(sample_rate)};
  MT::Lock lock(events_mutex);
  if (last_tick_end)
    last_tick_end += tick_duration;
  else
    last_tick_end = event_last_read;
  auto earliest = last_tick_end - tick_duration;
  auto i = 0u;
  sample_iterate(nsamples, {}, {},
                 tie(output),
                 [&](MIDI::Event& o)
  {
    o = MIDI::Event{};
    bool wrote = false;
    while (!events.empty() && events.front().t < earliest + sample_duration
           && (!wrote || (events.size() > (nsamples - i))))
    {
      if (wrote)
      {
        Log::Error log;
        log << "Supressing MIDI event (sample rate may be too low)" << endl;
      }
      o = events.front().e;
      events.pop();
      wrote = true;
    }
    earliest += sample_duration;
    ++i;
  });
}

//--------------------------------------------------------------------------
// Shut down
void ALSAIn::shutdown()
{
  Log::Detail log;
  log << "Shutting down ALSA MIDI input\n";

  running = false;
  if (thread) thread->join();

  if (midi_in) snd_rawmidi_close(midi_in);
  midi_in = nullptr;
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::DynamicModule module
{
  "alsa-in",
  "ALSA MIDI input",
  "midi",
  {
    { "device", &ALSAIn::device },
  },
  {},
  {
    { "output", &ALSAIn::output },
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ALSAIn, module)
