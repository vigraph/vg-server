//==========================================================================
// ViGraph dataflow module: midi/control/midi-linux-alsa/midi-linux-alsa.cc
//
// MIDI interface using Linux ALSA API
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include "../../midi-services.h"
#include <alsa/asoundlib.h>
#include "vg-midi.h"

using namespace ViGraph::Module::MIDI;

namespace {

const auto default_device = "default";

class MIDIInThread;  // forward

//==========================================================================
// MIDI interface implementation
class MIDIInterface: public Dataflow::Control,
                     public Distributor::EventObserver
{
private:
  snd_rawmidi_t *midi_in{nullptr};
  snd_rawmidi_t *midi_out{nullptr};
  unique_ptr<MIDIInThread> thread;
  bool running = false;
  ViGraph::MIDI::Reader reader;

  friend class MIDIInThread;
  void run();

  // Control virtuals
  void setup() override;
  void pre_tick(const TickData& td) override;
  void shutdown() override;

  // Event observer implementation
  void handle(const ViGraph::MIDI::Event& event) override;

public:
  string device = default_device;
  int channel_offset = 0;

  // Construct
  using Control::Control;
};

//==========================================================================
// MIDIIn thread
class MIDIInThread: public MT::Thread
{
private:
  MIDIInterface& control;

  void run() override
  { control.run(); }

public:
  MIDIInThread(MIDIInterface& _control): control(_control)
  { start(); }
};

//==========================================================================
// MIDIInterface implementation

//--------------------------------------------------------------------------
// Setup
void MIDIInterface::setup()
{
  Log::Streams log;

  // Input
  log.summary << "Opening MIDI input on ALSA device '" << device << "'\n";

  log.detail << "ALSA library version: " << SND_LIB_VERSION_STR << endl;

  auto in_status = snd_rawmidi_open(&midi_in, nullptr, device.c_str(),
                                    SND_RAWMIDI_SYNC | SND_RAWMIDI_NONBLOCK);
  auto out_status = snd_rawmidi_open(nullptr, &midi_out, device.c_str(),
                                     SND_RAWMIDI_SYNC | SND_RAWMIDI_NONBLOCK);
  if (in_status && out_status)
  {
    log.error << "Can't open MIDI device: in - " << snd_strerror(in_status)
              << " out - " << snd_strerror(out_status)
              << endl;
    return;
  }
  else if (out_status && !in_status)
  {
    log.summary << "Opened for input only (out: " << snd_strerror(out_status)
                << endl;
  }
  else if (in_status && !out_status)
  {
    log.summary << "Opened for output only (in: " << snd_strerror(in_status)
                << endl;
  }

  if (midi_in)
  {
    // Clear anything buffered
    snd_rawmidi_drop(midi_in);

    thread.reset(new MIDIInThread(*this));
    running = true;
  }

  // Output
  auto distributor = graph->find_service<Distributor>("midi", "distributor");
  if (distributor) distributor->register_event_observer(
                                ViGraph::MIDI::Event::Direction::out,
                                channel_offset + 1, channel_offset + 16,
                                ViGraph::MIDI::Event::Type::none, this);
}

//--------------------------------------------------------------------------
// Run background
void MIDIInterface::run()
{
  Log::Streams log;

  // Get FDs for poll
  const auto nfds = snd_rawmidi_poll_descriptors_count(midi_in);
  vector<struct pollfd> fds(nfds);
  snd_rawmidi_poll_descriptors(midi_in, fds.data(), nfds);

  // Filter for only input side
  vector<struct pollfd> ifds;
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

      for(auto i=0; i<n; i++)
        reader.add(bytes[i]);
    }
  }
}

//--------------------------------------------------------------------------
// Tick
void MIDIInterface::pre_tick(const TickData&)
{
  for(;;)
  {
    MIDI::Event event = reader.get();
    if (event.type == ViGraph::MIDI::Event::Type::none) break;
    event.channel += channel_offset;
    auto distributor = graph->find_service<Distributor>("midi", "distributor");
    if (distributor) distributor->handle_event(event);
  }
}

//--------------------------------------------------------------------------
// Handle event
void MIDIInterface::handle(const ViGraph::MIDI::Event& event)
{
  if (midi_out)
  {
    ViGraph::MIDI::Event e = event;
    e.channel -= channel_offset;
    vector<uint8_t> data;
    auto writer = ViGraph::MIDI::Writer{data};
    writer.write(event);
    snd_rawmidi_write(midi_out, &data[0], data.size());
  }
}

//--------------------------------------------------------------------------
// Shut down
void MIDIInterface::shutdown()
{
  Log::Detail log;
  log << "Shutting down MIDI input\n";

  auto distributor = graph->find_service<Distributor>("midi", "distributor");
  if (distributor) distributor->deregister_event_observer(this);

  running = false;
  if (thread) thread->join();

  if (midi_in) snd_rawmidi_close(midi_in);
  midi_in = nullptr;
  if (midi_out) snd_rawmidi_close(midi_out);
  midi_out = nullptr;
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "midi",
  "MIDI Interface",
  "MIDI Interface for Linux/ALSA",
  "midi",
  {
    { "device",  { "Device to listen to", Value::Type::text,
                    &MIDIInterface::device, false} },
    { "channel-offset", { "Offset to apply to channel number",
                          Value::Type::number, &MIDIInterface::channel_offset,
                          false } }
  },
  {}  // No controlled properties
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(MIDIInterface, module)
