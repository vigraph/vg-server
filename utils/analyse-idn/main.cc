//==========================================================================
// ViGraph vector graphics: analyse-idn.cc
//
// Utility to dump/analyse IDN input
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-idn.h"
#include "ot-text.h"
#include "ot-net.h"
#include "ot-misc.h"
#include <iostream>
#include <iomanip>

using namespace std;
using namespace ObTools;
using namespace ViGraph;

const int MAX_PACKET = 65536;

//--------------------------------------------------------------------------
// Usage
void usage(const string& path)
{
  cout << "ViGraph IDN analyser v0.1\n\n";
  cout << "Usage:\n";
  cout << "  " << path << " [options]\n\n";
  cout << "Options:\n";
  cout << "  -x --hex       Show all IDN bytes in hex\n";
  cout << "  -p --port <n>  Listen on UDP port <n> - default "
       << IDN::Hello::default_port << endl;
  cout << endl;
}


//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  bool show_hex = false;
  int port = IDN::Hello::default_port;

  for(int i=1; i<argc; i++)
  {
    string arg(argv[i]);
    if ((arg == "-x" || arg == "--hex"))
      show_hex = true;
    else if ((arg == "-p" || arg == "--port") && i<argc-1)
      port = Text::stoi(argv[++i]);
    else if (arg == "-?" || arg == "--help")
    {
      usage(argv[0]);
      return 0;
    }
    else
    {
      cerr << "No such option " << arg << endl;
      return 2;
    }
  }

  try
  {
    Net::UDPSocket udp(port);
    for(;;)
    {
      unsigned char buf[MAX_PACKET];
      auto len = udp.recv(buf, MAX_PACKET);
      cout << "Received " << len << " bytes\n";

      Channel::BlockReader reader(buf, len);
      IDN::Reader idn(reader);
      while (reader.get_offset() < (uint64_t)len)
      {
        try
        {
          IDN::HelloHeader hello;
          idn.read(hello);
          hello.dump(cout);

          if (hello.command == IDN::HelloHeader::Command::message)
          {
            IDN::Message message;
            idn.read(message);
            message.dump(cout);

            if (show_hex)
            {
              Misc::Dumper dumper(cout);
              dumper.dump(message.data.data(), message.data.size());
            }

            cout << endl;
          }
        }
        catch (runtime_error e)
        {
          cerr << "IDN parse failed: " << e.what() << endl;
        }
      }
    }
  }
  catch (Net::SocketError e)
  {
    cerr << "UDP socket failed: " << e.get_string() << endl;
    return 4;
  }

  return 0;
}




