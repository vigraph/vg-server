//==========================================================================
// ViGraph vector graphics: analyse-midi.cc
//
// Utility to dump/analyse MIDI input
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-midi.h"
#include <alsa/asoundlib.h>
#include <iostream>
#include <iomanip>

using namespace std;
using namespace ViGraph;

//--------------------------------------------------------------------------
// Usage
void usage(const string& path)
{
  cout << "ViGraph MIDI analyser v0.1\n\n";
  cout << "Usage:\n";
  cout << "  " << path << " [options] [device]\n\n";
  cout << "Options:\n";
  cout << "  -x --hex     Show all MIDI bytes in hex\n";
  cout << endl;
  cout << "[device] defaults to 'virtual' allowing it to be connected to\n";
}

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  bool show_hex = false;
  string device = "virtual";

  for(int i=1; i<argc; i++)
  {
    string arg(argv[i]);
    if ((arg == "-x" || arg == "--hex"))
      show_hex = true;
    else if (arg[0] == '-')
    {
      usage(argv[0]);
      return 2;
    }
    else device = arg;
  }

  MIDI::Reader reader;
  snd_rawmidi_t* midi{nullptr};
  auto status = snd_rawmidi_open(&midi, NULL, device.c_str(),
                                 SND_RAWMIDI_SYNC);
  if (status)
  {
    cerr << "Can't open MIDI input: " << snd_strerror(status) << endl;
    return 4;
  }

  for(;;)
  {
    unsigned char bytes[100];
    auto n = snd_rawmidi_read(midi, bytes, sizeof(bytes));
    if (n < 0)
    {
      cerr << "Can't read MIDI input: " << snd_strerror(n) << endl;
      return 4;
    }

    for(auto i=0; i<n; i++)
    {
      reader.add(bytes[i]);
      if (show_hex)
        cout << (i?' ':'[') << hex << setw(2) << setfill('0') << (int)bytes[i];
    }
    if (show_hex) cout << "]\n" << dec;

    MIDI::Event event = reader.get();
    if (event.type == MIDI::Event::Type::none) continue;

    cout << "CH " << (int)event.channel << "\t";

    switch (event.type)
    {
      case MIDI::Event::Type::none:
        break;

      case MIDI::Event::Type::note_off:
        cout << "ON\t" << (int)event.key << "\t@" << (int)event.value;
      break;

      case MIDI::Event::Type::note_on:
        cout << "OFF\t" << (int)event.key << "\t@" << (int)event.value;
        break;

      case MIDI::Event::Type::control_change:
        cout << "CONTROL\t" << (int)event.key << "\t=" << (int)event.value;
        break;

    }

    cout << endl;
  }

  return 0;
}




