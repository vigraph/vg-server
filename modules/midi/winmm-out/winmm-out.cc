//==========================================================================
// Write data to an WinMM MIDI device
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../midi-module.h"
#include "vg-midi.h"

namespace {

using namespace ViGraph::Dataflow;

const auto default_device = "default";
const auto sample_rate = 1000.0;

class WinMMOutThread;

//==========================================================================
// WinMMOut
class WinMMOut: public SimpleElement
{
private:
  HMIDIOUT midi_out{nullptr};
  unique_ptr<WinMMOutThread> thread;
  atomic<bool> running{false};
  MT::Queue<MIDIEvent> events;

  friend class WinMMOutThread;
  MT::Mutex zero_time_mutex;
  Time::Duration zero_time;
  void run();

  // Source/Element virtuals
  void setup(const SetupContext& context) override;
  void tick(const TickData& td) override;
  void shutdown() override;

  // Clone
  WinMMOut *create_clone() const override
  {
    return new WinMMOut{module};
  }

public:
  using SimpleElement::SimpleElement;

  Setting<string> device{default_device};

  Input<MIDIEvents> input;
};

//==========================================================================
// WinMMOut thread
class WinMMOutThread: public MT::Thread
{
private:
  WinMMOut& out;

  void run() override
  { out.run(); }

public:
  WinMMOutThread(WinMMOut& _out): out{_out} {}
};

//--------------------------------------------------------------------------
// Get devices
vector<string> get_devices()
{
  auto devices = vector<string>{};
  auto seen_count = map<string, int>{};
  const auto num_out = midiOutGetNumDevs();
  auto out_caps = MIDIOUTCAPS{};
  for (auto i = 0u; i < num_out; ++i)
  {
    if (midiOutGetDevCaps(i, &out_caps, sizeof(out_caps)) == MMSYSERR_NOERROR)
    {
      auto name = string{out_caps.szPname};
      auto seen = ++seen_count[name];
      if (seen > 1)
        name += " (" + Text::itos(seen) + ")";
      devices.emplace_back(name);
    }
  }
  return devices;
}

//--------------------------------------------------------------------------
// Setup
void WinMMOut::setup(const SetupContext& context)
{
  SimpleElement::setup(context);

  Log::Streams log;
  shutdown();

  input.set_sample_rate(sample_rate);

  const auto devices = get_devices();
  auto d = 0u;
  for (; d < devices.size(); ++d)
  {
    if (device == devices[d])
      break;
  }
  if (d >= devices.size())
  {
    log.error << "Unknown MIDI out device: '" << device << "'" << endl;
    log.summary << "Available MIDI out devices:" << endl;
    if (devices.empty())
    {
      log.summary << "  No MIDI out devices found." << endl;
    }
    else
    {
      for (const auto& d: devices)
      {
        log.summary << "  '" << d << "'" << endl;
      }
    }
  }


  log.summary << "Opening MIDI output on WinMM device '" << device << "'\n";

  const auto status = midiOutOpen(&midi_out, d,
                                  reinterpret_cast<DWORD_PTR>(nullptr),
                                  reinterpret_cast<DWORD_PTR>(nullptr),
                                  CALLBACK_NULL);
  if (status)
  {
    log.error << "Can't open MIDI device: " << get_winmm_error(status) << endl;
    midi_out = nullptr;
    return;
  }

  thread.reset(new WinMMOutThread(*this));
  running = true;
  thread->start();
}

//--------------------------------------------------------------------------
// Run background
void WinMMOut::run()
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
      auto d = DWORD{};
      for (auto i = 0u; i < data.size() && i < sizeof(DWORD); ++i)
        d |= data[i] << (i * 8);
      auto result = midiOutShortMsg(midi_out, d);
      if (result != MMSYSERR_NOERROR)
      {
        Log::Error log;
        log << "MIDI send error: " << get_winmm_error(result) << endl;
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
void WinMMOut::tick(const TickData& td)
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
void WinMMOut::shutdown()
{
  Log::Detail log;
  log << "Shutting down WinMM MIDI output\n";

  running = false;
  if (thread) thread->join();

  if (midi_out) midiOutClose(midi_out);
  midi_out = nullptr;
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::DynamicModule module
{
  "winmm-out",
  "WinMM MIDI output",
  "midi",
  {
    { "device", &WinMMOut::device },
  },
  {
    { "input", &WinMMOut::input },
  },
  {},
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(WinMMOut, module)
