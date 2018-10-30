//==========================================================================
// ViGraph vector graphics: sign-licence.cc
//
// Utility to sign licences
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-licence.h"
#include "ot-log.h"
#include "ot-crypto.h"
#include <fstream>

using namespace std;
using namespace ObTools;
using namespace ViGraph;

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  if (argc < 3)
  {
    cout << "Usage:\n";
    cout << "  sign-licence <licence-path> <private key path> [<pass-phrase>]\n";
    return 2;
  }

  auto chan_out = new Log::StreamChannel{&cout};
  Log::logger.connect(chan_out);
  Log::Streams log;

  Crypto::Library library;

  Licence::File licence(argv[1], log.error);
  ifstream pkf(argv[2]);
  string private_key;

  char c;
  while (!pkf.eof() && pkf.get(c)) private_key += c;

  string passphrase;
  if (argc > 3) passphrase = argv[3];

  // Sign it
  if (licence.sign(private_key, passphrase))
  {
    log.summary << "Signed licence OK\n";

    // Check it again
    Licence::File licence2(argv[1]);
    if (!licence2.validate())
    {
      log.error << "Re-check of licence failed!\n";
      return 2;
    }
  }
  else
  {
    log.error << "Can't sign licence\n";
    return 2;
  }

  return 0;
}




