//==========================================================================
// ViGraph dataflow module: linux/controls/midi-in/midi-in.cc
//
// MIDI input control
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include "ot-web.h"
#include <alsa/asoundlib.h>

namespace {

const auto default_device = "default";

class MIDIInThread;  // forward

//==========================================================================
// MIDIIn control
class MIDIInControl: public Dataflow::Control
{
  int base{0};
  int modulus{0};

  snd_rawmidi_t* midi{nullptr};
  unique_ptr<MIDIInThread> thread;
  bool running;
  vector<unsigned char> buffer;

  friend class MIDIInThread;
  void run();
  void check_messages();
  void handle_note_on(int chan, int key, int velocity);
  void handle_note_off(int chan, int key, int velocity);

  // Control virtuals
  void shutdown() override;

public:
  // Construct
  MIDIInControl(const Module *module, const XML::Element& config);
};

//==========================================================================
// MIDIIn thread
class MIDIInThread: public MT::Thread
{
private:
  MIDIInControl& control;

  void run() override
  { control.run(); }

public:
  MIDIInThread(MIDIInControl& _control): control(_control)
  { start(); }
};

//==========================================================================
// MIDIIn control implementation

//--------------------------------------------------------------------------
// Construct from XML:
//   <midi-in device="hw:0:1"/>
MIDIInControl::MIDIInControl(const Module *module,
                             const XML::Element& config):
  Element(module, config), Control(module, config)
{
  Log::Streams log;
  const auto& device = config.get_attr("device", default_device);
  log.summary << "Opening MIDI input on ALSA device '" << device << "'\n";
  base = config.get_attr_int("base");
  if (base)
    log.detail << "MIDI note base: " << base << endl;
  modulus = config.get_attr_int("modulus");
  if (modulus)
    log.detail << "MIDI note modulus: " << modulus << endl;

  log.detail << "ALSA library version: " << SND_LIB_VERSION_STR << endl;

   auto status = snd_rawmidi_open(&midi, NULL, device.c_str(),
                                  SND_RAWMIDI_SYNC);
   if (status)
   {
     log.error << "Can't open MIDI input: " << snd_strerror(status) << endl;
     return;
   }

   thread.reset(new MIDIInThread(*this));
   running = true;
}

//--------------------------------------------------------------------------
// Run background
void MIDIInControl::run()
{
  Log::Streams log;
  while (running)
  {
    unsigned char bytes[100];
    auto n = snd_rawmidi_read(midi, bytes, sizeof(bytes));
    if (n < 0)
    {
      log.error << "Can't read MIDI input: " << snd_strerror(n) << endl;
      continue;
    }

    for(auto i=0; i<n; i++)
      buffer.push_back(bytes[i]);

    check_messages();
  }
}

//--------------------------------------------------------------------------
// Check for complete messages in the buffer
void MIDIInControl::check_messages()
{
  // If top bit not set when we enter here, we're out of sync or in a
  // system exclusive message - either way, just drop them
  while (!buffer.empty() && !(buffer[0] & 0x80))
    buffer.erase(buffer.begin());

  if (!buffer.empty())
  {
    const auto first = buffer[0];
    const auto cmd = (first >> 4) & 7; // We know top bit is set, ignore it
    const auto chan = (first & 0xf) + 1;
    auto erase = int{0};

    switch (cmd)
    {
      case 0:  // Note off
        if (buffer.size() >= 3)
        {
          handle_note_off(chan, buffer[1], buffer[2]);
          erase = 3;
        }
      break;

      case 1:  // Note on
        if (buffer.size() >= 3)
        {
          handle_note_on(chan, buffer[1], buffer[2]);
          erase = 3;
        }
      break;
    }

    if (erase) buffer.erase(buffer.begin(), buffer.begin()+erase);
  }
}

//--------------------------------------------------------------------------
// Handle note on
void MIDIInControl::handle_note_on(int chan, int key, int velocity)
{
  Log::Detail log;
  log << "MIDI " << chan << ": key " << key << " ON @" << velocity << endl;
  key -= base;
  if (key >= 0)
  {
    if (modulus) key %= modulus;
    send(Dataflow::Value(key));
  }
}

//--------------------------------------------------------------------------
// Handle note off
void MIDIInControl::handle_note_off(int chan, int key, int /*velocity*/)
{
  Log::Detail log;
  log << "MIDI " << chan << ": key " << key << " OFF\n";
}

//--------------------------------------------------------------------------
// Shut down
void MIDIInControl::shutdown()
{
  Log::Detail log;
  log << "Shutting down MIDI input\n";

  running = false;
  if (thread)
  {
    thread->cancel();
    thread->join();
  }

  if (midi) snd_rawmidi_close(midi);
  midi = nullptr;
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "midi-in",
  "MIDI Input",
  "MIDI Input for Linux/ALSA",
  "linux",
  {
    { "device",  { {"Device to listen to", "default"}, Value::Type::text,
                                                       "@device" } },
    { "base",    { {"Base note value", "0"},           Value::Type::number,
                                                       "@base" } },
    { "modulus", { {"Note value modulus", "0"},        Value::Type::number,
                                                       "@modulus" } }
  },
  { { "", { "Key code", "key", Value::Type::number }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(MIDIInControl, module)
