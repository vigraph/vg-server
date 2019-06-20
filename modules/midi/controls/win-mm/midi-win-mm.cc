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

  // Get list of devices
  struct IODevice
  {
    int in = -1;
    int out = -1;
  };
  static map<string, IODevice> get_devices();


  // Get error string
  static string get_error(MMRESULT result);

public:
  string device;
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

  const auto devices = get_devices();
  const auto dev = devices.find(device);
  if (dev == devices.end())
  {
    log.error << "Unknown MIDI device: '" << device << "'" << endl;
    log.summary << "Available MIDI devices:" << endl;
    if (devices.empty())
    {
      log.summary << "  No MIDI devices found." << endl;
    }
    else
    {
      for (const auto& d: devices)
      {
        log.summary << "  '" << d.first << "' ("
                    << (d.second.in >= 0 ? "in" : "")
                    << (d.second.out >= 0 ?
                        string{d.second.in >= 0 ? "/" : ""} + "out" : "")
                    << ")" << endl;
      }
    }
  }

  if (dev->second.in >= 0)
  {
    // Input
    log.summary << "Opening MIDI input on device '" << device << "'\n";

    auto status = midiInOpen(&midi_in, dev->second.in,
                             reinterpret_cast<DWORD_PTR>(callback),
                             reinterpret_cast<DWORD_PTR>(this),
                             CALLBACK_FUNCTION);
    if (status)
    {
      log.error << "Can't open MIDI input: " << get_error(status) << endl;
      midi_in = nullptr;
    }
    else
    {
      midiInStart(midi_in);
    }
  }

  if (dev->second.out >= 0)
  {
    // Output
    log.summary << "Opening MIDI output on device '" << device << "'\n";

    auto status = midiOutOpen(&midi_out, dev->second.out,
                              reinterpret_cast<DWORD_PTR>(nullptr),
                              reinterpret_cast<DWORD_PTR>(nullptr),
                              CALLBACK_NULL);
    if (status)
    {
      log.error << "Can't open MIDI output: " << get_error(status) << endl;
      midi_out = nullptr;
    }
  }

  auto distributor = graph->find_service<Distributor>("midi", "distributor");
  if (distributor) distributor->register_event_observer(
                                ViGraph::MIDI::Event::Direction::out,
                                channel_offset + 1, channel_offset + 16,
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
  if (midi_out)
  {
    ViGraph::MIDI::Event e = event;
    e.channel -= channel_offset;
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
      log << "MIDI send error: " << get_error(result) << endl;
    }
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

  if (midi_in) midiInClose(midi_in);
  midi_in = nullptr;
  if (midi_out) midiOutClose(midi_out);
  midi_out = nullptr;
}

//--------------------------------------------------------------------------
// Get list of MIDI devices
map<string, MIDIInterface::IODevice> MIDIInterface::get_devices()
{
  auto devices = map<string, IODevice>{};

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
      devices[name].in = i;
    }
  }

  seen_count.clear();
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
      devices[name].out = i;
    }
  }

  return devices;
}

//--------------------------------------------------------------------------
// Get error string
string MIDIInterface::get_error(MMRESULT result)
{
  switch (result)
  {
    case MMSYSERR_ALLOCATED:
      return "The specified resource is already allocated";
    case MMSYSERR_BADDEVICEID:
      return "The specified device identifier is out of range";
    case MMSYSERR_INVALFLAG:
      return "The flags specified by dwFlags are invalid";
    case MMSYSERR_INVALPARAM:
      return "The specified pointer or structure is invalid";
    case MMSYSERR_NOMEM:
      return "The system is unable to allocate or lock memory";
    case MIDIERR_BADOPENMODE:
      return "The application sent a message without a status byte "
             "to a stream handle";
    case MIDIERR_NOTREADY:
      return "The hardware is busy with other data";
    case MMSYSERR_INVALHANDLE:
      return "The specified device handle is invalid";
    default:
      return string{"Unknown error: "} + Text::itos(result);
  }
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
