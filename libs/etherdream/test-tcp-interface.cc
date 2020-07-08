//==========================================================================
// ViGraph Ether Dream protocol library: test-tcp-interface.cc
//
// Tests for TCP interface
//
// By default tests against an internal fake, but
// also accepts a hostname for test with a real one
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-etherdream.h"
#include "ot-log.h"

#include <gtest/gtest.h>
#include <sstream>

namespace {

using namespace ViGraph;
using namespace ViGraph::EtherDream;
using namespace ViGraph::Geometry;
using namespace ObTools;
using namespace std;

string host{"localhost"};
int port{7765};

// OK status responses
const vector<uint8_t> ok_response_idle
{
  'a',    // ACK
  0,      // command

  // status:
  0,      // protocol
  0,      // light engine state: ready
  0,      // playback state: idle
  0,      // source: network
  0, 0,   // light engine flags: none
  1, 0,   // playback flags: underflow
  0, 1,   // source flags = 256
  1, 4,   // buffer fullness = 1025
  0, 0, 1, 0,  // point rate = 63336
  0, 0, 0, 1   // point count = 16M
};

const vector<uint8_t> ok_response_prepared
{
  'a',    // ACK
  0,      // command

  // status:
  0,      // protocol
  0,      // light engine state: ready
  1,      // playback state: prepared
  0,      // source: network
  0, 0,   // light engine flags: none
  1, 0,   // playback flags: underflow
  0, 1,   // source flags = 256
  1, 4,   // buffer fullness = 1025
  0, 0, 1, 0,  // point rate = 63336
  0, 0, 0, 1   // point count = 16M
};

// Fake device
class FakeDevice: public Net::TCPServer
{
  virtual void process(Net::TCPSocket &socket, Net::EndPoint client)
  {
    Log::Streams log;
    log.detail << "Fake server got connection from " << client << endl;

    try
    {
      // Send initial 'response'
      socket.write(ok_response_idle.data(), ok_response_idle.size());

      while (!!socket)
      {
        // Read whatever they send
        string s;
        socket.read(s);
        log.detail << "Fake server received: '" << s << "'\n";

        // Send back a response
        auto response = &ok_response_idle;

        switch (s[0])
        {
          case 'p':
            response = &ok_response_prepared;
            break;
        }

        socket.write(response->data(), response->size());
      }
    }
    catch (Net::SocketError se)
    {
      log.error << "Fake server stopped: " << se << endl;
    }
  }

public:
  FakeDevice(int _port): Net::TCPServer(_port) {}
};

TEST(TCPInterfaceTest, test_interface_startup_and_prepare)
{
  FakeDevice device(port);
  Net::TCPServerThread thread(device);

  TCPInterface intf(Net::EndPoint(host, port));
  EXPECT_TRUE(intf.start());
  EXPECT_TRUE(intf.get_ready());
  device.shutdown();
}

} // anonymous namespace

int main(int argc, char **argv)
{
  for(int i=1; i<argc; i++)
  {
    string arg(argv[i]);

    if (arg == "-v")
    {
      auto chan_out = new Log::StreamChannel{&cout};
      Log::logger.connect(chan_out);
    }
    else host = arg;
  }

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
