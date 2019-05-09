//==========================================================================
// ViGraph dataflow module: midi/control/midi-win-mm/midi-win-mm.cc
//
// MIDI interface using Windows Multimedia API
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include "../../midi-services.h"
#include <windows.h>
#include <mmsystem.h>
#include "vg-midi.h"

using namespace ViGraph::Module::MIDI;

namespace {

const auto default_device = 0;

//==========================================================================
// MIDI interface implementation
class MIDIInterface: public Dataflow::Control,
                     public Distributor::EventObserver
{
private:
  HMIDIIN midi_in{nullptr};
  HMIDIOUT midi_out{nullptr};
  bool running = false;
  ViGraph::MIDI::Reader reader;

  // Control virtuals
  void setup() override;
  void pre_tick(const TickData& td) override;
  void shutdown() override;

  // Event observer implementation
  void handle(const ViGraph::MIDI::Event& event) override;

public:
  int device = default_device;
  int channel_offset = 0;

  // Construct
  using Control::Control;

  // Receive data
  void in_data(uint32_t data);
};

//--------------------------------------------------------------------------
// Callback
void callback(HMIDIIN, UINT msg, DWORD_PTR midi_p, DWORD_PTR data,
              DWORD_PTR /*timestamp*/)
{
  if (msg == MIM_DATA)
  {
    auto midi = reinterpret_cast<MIDIInterface *>(midi_p);
    if (midi)
      midi->in_data(data);
  }
}

//--------------------------------------------------------------------------
// Setup
void MIDIInterface::setup()
{
  Log::Streams log;

  // Input
  log.summary << "Opening MIDI input on device '" << device << "'\n";

  auto status = midiInOpen(&midi_in, device,
                           reinterpret_cast<DWORD_PTR>(callback),
                           reinterpret_cast<DWORD_PTR>(this),
                           CALLBACK_FUNCTION);
  if (status)
  {
    log.error << "Can't open MIDI input" << endl;
    return;
  }
  midiInStart(midi_in);

  // Output
  status = midiOutOpen(&midi_out, device,
                       reinterpret_cast<DWORD_PTR>(nullptr),
                       reinterpret_cast<DWORD_PTR>(nullptr),
                       CALLBACK_NULL);
  if (status)
  {
    log.error << "Can't open MIDI output" << endl;
    return;
  }

  auto distributor = graph->find_service<Distributor>("midi", "distributor");
  if (distributor) distributor->register_event_observer(
                                ViGraph::MIDI::Event::Direction::out,
                                channel_offset, channel_offset + 15,
                                ViGraph::MIDI::Event::Type::none, this);
}

//--------------------------------------------------------------------------
// Receive data
void MIDIInterface::in_data(uint32_t data)
{
  reader.add(data);
  reader.add(data >> 8);
  reader.add(data >> 16);
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
  ViGraph::MIDI::Event e = event;
  e.channel -= channel_offset;
  vector<uint8_t> data;
  auto writer = ViGraph::MIDI::Writer{data};
  writer.write(event);
  auto d = DWORD{};
  for (auto i = 0u; i < data.size() && i < sizeof(DWORD); ++i)
    d |= data[i] << (i * 8);
  auto status = midiOutShortMsg(midi_out, d);
}

//--------------------------------------------------------------------------
// Shut down
void MIDIInterface::shutdown()
{
  Log::Detail log;
  log << "Shutting down MIDI input\n";

  auto distributor = graph->find_service<Distributor>("midi", "distributor");
  if (distributor) distributor->deregister_event_observer(this);

  if (midi_in) midiInClose(midi_in);
  midi_in = nullptr;
  if (midi_out) midiOutClose(midi_out);
  midi_out = nullptr;
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "midi",
  "MIDI Interface",
  "MIDI Interface for Windows Multimedia",
  "midi",
  {
    { "device",  { "Device to listen to", Value::Type::number,
                    &MIDIInterface::device, false} },
    { "channel-offset", { "Offset to apply to channel number",
                          Value::Type::number, &MIDIInterface::channel_offset,
                          false } }
  },
  {}  // No controlled properties
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(MIDIInterface, module)
