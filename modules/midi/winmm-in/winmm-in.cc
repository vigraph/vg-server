//==========================================================================
// ViGraph dataflow module: midi/winmm-in/winmm-in.cc
//
// Read data from an WinMM MIDI device
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../midi-module.h"
#include "vg-midi.h"

namespace {

using namespace ViGraph::Dataflow;

const auto default_device = "default";

class WinMMInThread;

//==========================================================================
// WinMMIn
class WinMMIn: public SimpleElement
{
private:
  HMIDIIN midi_in{nullptr};
  ViGraph::MIDI::Reader reader;
  MT::Mutex events_mutex;
  queue<MIDIEvent> events;
  Time::Duration event_last_read;
  Time::Duration last_tick_end;

  void run();

  // Source/Element virtuals
  void setup(const SetupContext& context) override;
  void tick(const TickData& td) override;
  void shutdown();

  // Clone
  WinMMIn *create_clone() const override
  {
    return new WinMMIn{module};
  }

public:
  using SimpleElement::SimpleElement;

  void receive_data(uint32_t data, const Time::Duration& time);

  Setting<string> device{default_device};

  Output<MIDIEvents> output;

  ~WinMMIn() { shutdown(); }
};

//--------------------------------------------------------------------------
// Callback
void callback(HMIDIIN, UINT msg, DWORD_PTR midi_p, DWORD_PTR data,
              DWORD_PTR /*timestamp*/)
{
  if (msg == MIM_DATA)
  {
    auto midi = reinterpret_cast<WinMMIn *>(midi_p);
    if (midi)
      midi->receive_data(data, Time::Duration::clock());
  }
}

//--------------------------------------------------------------------------
// Get devices
vector<string> get_devices()
{
  auto devices = vector<string>{};
  auto seen_count = map<string, int>{};
  const auto num_in = midiInGetNumDevs();
  auto in_caps = MIDIINCAPS{};
  for (auto i = 0u; i < num_in; ++i)
  {
    if (midiInGetDevCaps(i, &in_caps, sizeof(in_caps)) == MMSYSERR_NOERROR)
    {
      auto name = string{in_caps.szPname};
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
void WinMMIn::setup(const SetupContext& context)
{
  SimpleElement::setup(context);

  Log::Streams log;
  shutdown();

  const auto devices = get_devices();
  auto d = 0u;
  for (; d < devices.size(); ++d)
  {
    if (device == devices[d])
      break;
  }
  if (d >= devices.size())
  {
    log.error << "Unknown MIDI in device: '" << device << "'" << endl;
    log.summary << "Available MIDI in devices:" << endl;
    if (devices.empty())
    {
      log.summary << "  No MIDI in devices found." << endl;
    }
    else
    {
      for (const auto& d: devices)
      {
        log.summary << "  '" << d << "'" << endl;
      }
    }
    return;
  }

  log.summary << "Opening MIDI input on WinMM device '" << device << "'\n";

  const auto status = midiInOpen(&midi_in, d,
                                 reinterpret_cast<DWORD_PTR>(callback),
                                 reinterpret_cast<DWORD_PTR>(this),
                                 CALLBACK_FUNCTION);
  if (status)
  {
    log.error << "Can't open MIDI device: " << get_winmm_error(status) << endl;
    midi_in = nullptr;
    return;
  }

  midiInStart(midi_in);
}

//--------------------------------------------------------------------------
// Receive data
void WinMMIn::receive_data(uint32_t data, const Time::Duration& time)
{
  MT::Lock lock{events_mutex};
  event_last_read = time;
  reader.add(data);
  reader.add(data >> 8);
  reader.add(data >> 16);
  auto ev = reader.get();
  while (ev.type != MIDI::Event::Type::none)
  {
    events.emplace(time, ev);
    ev = reader.get();
  }
}

//--------------------------------------------------------------------------
// Process some data
void WinMMIn::tick(const TickData& td)
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
  if (!sample_rate)
  {
    while (!events.empty())
      events.pop();
    auto o = output.get_buffer(td);
    return;
  }
  auto earliest = last_tick_end - tick_duration;
  auto sample_time = Time::Duration{td.start};
  sample_iterate(td, nsamples, {}, {},
                 tie(output),
                 [&](MIDIEvents& o)
  {
    o = MIDIEvents{};
    while (!events.empty() && events.front().time < earliest + sample_duration)
    {
      o.emplace_back(events.front());
      o.back().time = sample_time;
      events.pop();
    }
    sample_time += sample_duration;
    earliest += sample_duration;
  });
}

//--------------------------------------------------------------------------
// Shut down
void WinMMIn::shutdown()
{
  Log::Detail log;
  log << "Shutting down WinMM MIDI input\n";

  if (midi_in) midiInClose(midi_in);
  midi_in = nullptr;
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::DynamicModule module
{
  "winmm-in",
  "WinMM MIDI input",
  "midi",
  {
    { "device", &WinMMIn::device },
  },
  {},
  {
    { "output", &WinMMIn::output },
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(WinMMIn, module)
