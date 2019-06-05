//==========================================================================
// ViGraph IDN stream library: hello.cc
//
// IDN Hello header implementation
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-idn.h"

namespace ViGraph { namespace IDN {

// Dump to given stream
void HelloHeader::dump(ostream& out)
{
  out << "Hello (flags " << (int)flags << ", sequence "
      << sequence << "): ";

  switch (command)
  {
    case HelloHeader::Command::void_:
      out << "void\n";
      break;

    case HelloHeader::Command::ping_request:
      out << "ping request\n";
      break;

    case HelloHeader::Command::ping_reply:
      out << "ping reply\n";
      break;

    case HelloHeader::Command::scan_request:
      out << "scan request\n";
      break;

    case HelloHeader::Command::scan_reply:
      out << "scan reply\n";
      break;

    case HelloHeader::Command::message:
      out << "message\n";
      break;
  }
}


}} // namespaces
