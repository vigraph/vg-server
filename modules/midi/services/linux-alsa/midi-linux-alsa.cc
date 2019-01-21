//==========================================================================
// ViGraph dataflow module: midi/services/midi-linux-alsa/midi-linux-alsa.cc
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
class MIDIInterfaceImpl: public Dataflow::Service, public Interface
{
  snd_rawmidi_t* midi{nullptr};
  unique_ptr<MIDIInThread> thread;
  bool running;
  ViGraph::MIDI::Reader reader;

  friend class MIDIInThread;
  void run();

  // Service interface
  void tick(const TickData& td) override;
  void shutdown() override;

  // MIDI interface implementation
  void register_event_observer(int channel,
                               ViGraph::MIDI::Event::Type type,
                               EventObserver *observer) override;
  void deregister_event_observer(EventObserver *observer) override;

  // Event observers
  struct Observer
  {
    int channel;
    ViGraph::MIDI::Event::Type type;
    EventObserver *observer;

    Observer(int _channel, ViGraph::MIDI::Event::Type _type,
             EventObserver *_observer):
      channel(_channel), type(_type), observer(_observer) {}
  };

  list<Observer> observers;

public:
  // Construct
  MIDIInterfaceImpl(const Dataflow::Module *module,
                    const XML::Element& config);
};

//==========================================================================
// MIDIIn thread
class MIDIInThread: public MT::Thread
{
private:
  MIDIInterfaceImpl& control;

  void run() override
  { control.run(); }

public:
  MIDIInThread(MIDIInterfaceImpl& _control): control(_control)
  { start(); }
};

//==========================================================================
// MIDIInterface implementation

//--------------------------------------------------------------------------
// Construct from XML:
//   <midi device='default'/>
MIDIInterfaceImpl::MIDIInterfaceImpl(const Dataflow::Module *module,
                                     const XML::Element& config):
  Service(module, config)
{
  Log::Streams log;
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

  thread.reset(new MIDIInThread(*this));
  running = true;
}

//--------------------------------------------------------------------------
// Register an event handler - channel=0 means all (Omni)
void MIDIInterfaceImpl::register_event_observer(int channel,
                                                ViGraph::MIDI::Event::Type type,
                                                EventObserver *observer)
{
  observers.push_back(Observer(channel, type, observer));
}

//--------------------------------------------------------------------------
// Deregister an event observer for all events
void MIDIInterfaceImpl::deregister_event_observer(EventObserver *observer)
{
  for(auto p=observers.begin(); p!=observers.end();)
  {
    Observer& o = *p;
    if (o.observer == observer)
      p = observers.erase(p);
    else
      ++p;
  }
}

//--------------------------------------------------------------------------
// Run background
void MIDIInterfaceImpl::run()
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
void MIDIInterfaceImpl::tick(const TickData&)
{
  for(;;)
  {
    MIDI::Event event = reader.get();
    if (event.type == ViGraph::MIDI::Event::Type::none) break;

    // Send event to all interested observers
    for(const auto& o: observers)
    {
      if ((!o.channel || o.channel == event.channel) // channel 0 is wildcard
          && o.type == event.type)
        o.observer->handle(event);
    }
  }
}

//--------------------------------------------------------------------------
// Shut down
void MIDIInterfaceImpl::shutdown()
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
    { "device",  { {"Device to listen to", "default"}, Value::Type::text,
                                                       "@device" } }
  }
};

} // anon

VIGRAPH_ENGINE_SERVICE_MODULE_INIT(MIDIInterfaceImpl, module)
