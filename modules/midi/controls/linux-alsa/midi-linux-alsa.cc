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
class MIDIInterface: public Dataflow::Control
{
  shared_ptr<Distributor> distributor;
  int channel_offset{0};

  snd_rawmidi_t* midi{nullptr};
  unique_ptr<MIDIInThread> thread;
  bool running;
  ViGraph::MIDI::Reader reader;

  friend class MIDIInThread;
  void run();

  // Control virtuals
  void configure(const File::Directory&, const XML::Element& config) override;
  void tick(const TickData& td) override;
  void shutdown() override;

public:
  // Construct
  MIDIInterface(const Dataflow::Module *module,
                    const XML::Element& config);
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
// Construct from XML:
//   <midi device='default' channel-offset="16"/>
MIDIInterface::MIDIInterface(const Dataflow::Module *module,
                             const XML::Element& config):
  Element(module, config), Control(module, config, true)  // no props
{
  channel_offset = config.get_attr_int("channel-offset");
}

//--------------------------------------------------------------------------
// Configure from XML (once we have the engine)
void MIDIInterface::configure(const File::Directory&,
                              const XML::Element& config)
{
  Log::Streams log;
  auto& engine = graph->get_engine();
  distributor = engine.get_service<Distributor>("midi-distributor");
  if (!distributor)
  {
    log.error << "No <midi-distributor> service loaded\n";
    return;
  }

  const auto& device = config.get_attr("device", default_device);
  log.summary << "Opening MIDI input on ALSA device '" << device << "'\n";

  log.detail << "ALSA library version: " << SND_LIB_VERSION_STR << endl;

  auto status = snd_rawmidi_open(&midi, NULL, device.c_str(),
                                 SND_RAWMIDI_SYNC | SND_RAWMIDI_NONBLOCK);
  if (status)
  {
    log.error << "Can't open MIDI input: " << snd_strerror(status) << endl;
    return;
  }

  // Clear anything buffered
  snd_rawmidi_drop(midi);

  thread.reset(new MIDIInThread(*this));
  running = true;
}

//--------------------------------------------------------------------------
// Run background
void MIDIInterface::run()
{
  Log::Streams log;

  // Get FDs for poll
  const auto nfds = snd_rawmidi_poll_descriptors_count(midi);
  vector<struct pollfd> fds(nfds);
  snd_rawmidi_poll_descriptors(midi, fds.data(), nfds);

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
      auto n = snd_rawmidi_read(midi, bytes, sizeof(bytes));
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
void MIDIInterface::tick(const TickData&)
{
  for(;;)
  {
    MIDI::Event event = reader.get();
    if (event.type == ViGraph::MIDI::Event::Type::none) break;
    event.channel += channel_offset;
    if (distributor) distributor->handle_event(event);
  }
}

//--------------------------------------------------------------------------
// Shut down
void MIDIInterface::shutdown()
{
  Log::Detail log;
  log << "Shutting down MIDI input\n";

  running = false;
  if (thread) thread->join();

  if (midi) snd_rawmidi_close(midi);
  midi = nullptr;
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
    { "device",  { {"Device to listen to", "default"},
          Value::Type::text, "@device" } },
    { "channel-offset",  { "Offset to apply to channel number",
          Value::Type::number, "@channel-offset" } }
  },
  {}  // No controlled properties
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(MIDIInterface, module)
