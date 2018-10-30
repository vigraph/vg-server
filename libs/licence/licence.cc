//==========================================================================
// ViGraph vector graphics licence library: licence.cc
//
// Implementation of licence file
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-licence.h"
#include "ot-log.h"
#include "ot-misc.h"
#include "ot-text.h"
#include "ot-time.h"
#include "ot-crypto.h"
#include <errno.h>

namespace ViGraph { namespace Licence {

//--------------------------------------------------------------------------
// Hardwired public key
static const char *public_key =
  R"(-----BEGIN PUBLIC KEY-----
MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAwkgr42mZ0r401gEACX4W
2jbeLppN/MiMQRSgw6ck/TIHVWCPSdX4x/OM3DpPqW4bcxJtJcIlUssl7pMRBlCt
iPvlAFm5f/ZMxD+ESR8kt+rJMhceUZv5kXSsAzc6q4xj2OGxfuZloekW1aoD7mD3
Zu3ztQ2AjfnwTmYVkSdKUlOnXFCsKyidDOamSasBA7SfsQAbLFbvSNbHp33vZbgG
yhmMPeqw4OqyLEzY2SZ5igqQdo7cgJK0u5/7yfBvRDJhyxf2CK9vGvwp3vysuFRF
42ejJevlS4jo9G8pYKjTZomsNl06FQPlxAWUkumNO2MO6g4JvThVnF5CHSEqeMTg
vama5aKvt748Z6CVYO26kysnTbxdEINxQRB/frJA9YHg3bDflmpK1JkWroTO1zt/
DxwWd9nNMo10kfV5HHPf66o6bW/ZKtCDjhibwuH3M28uGbRlJWm5oeh8SlQWNjjn
Ws7+vycXI58L6x1oByI4hgvzUR7PG3JcTINoKelmEKLv+cMpK/lOsVR2oZBZPjar
bosV1Eh/Nv+3nYy0nCojEr7kCuhJ2CIZ62LxrB34xlm0GvOrI01Qejmdl96Zp5Qj
AiUC5TD8LicS+kJnhBxEFt3mHP2QFOJxDc1twCha4fSkVFInVX896tO5eYExMXPd
ewWQVWMJ+Ojifu7bDT/cGPMCAwEAAQ==
-----END PUBLIC KEY-----)";

//--------------------------------------------------------------------------
// Constructor
File::File(const string& fn, ostream& _serr): XML::Configuration(fn, _serr)
{
  if (!read("licence"))
    serr << "Can't read licence file " << fn << endl;
}

//--------------------------------------------------------------------------
// Verify validity of licence file - checks signature and window
// Independent of useability on current machine
bool File::validate()
{
  XML::Element& root = get_root();
  if (!root) return false;  // Failed to read

  // Get current signature, then clear it
  XML::Element& sig_e = root.get_child("signature");
  if (!sig_e)
  {
    serr << "No signature element in licence\n";
    return false;
  }
  string sig = sig_e.content;
  sig_e.content.clear();

  // Export XML as text
  string text = root.to_string();

  // Get SHA1
  Crypto::SHA1 sha1;
  sha1.update(text.c_str(), text.size());
  unsigned char digest[Crypto::SHA1::DIGEST_LENGTH];
  sha1.get_result(digest);

  // Get RSA public key
  Crypto::RSA rsa;
  rsa.key.set(public_key, false);
  if (!rsa.key.valid) return false;

  // Extract sig from base64
  Text::Base64 base64;
  size_t csize = rsa.cipher_size();
  unsigned char *cipher = new unsigned char[csize];
  if (base64.decode(sig, cipher, csize) != csize)
  {
    serr << "Licence signature has bad length\n";
    return false;
  }

  // Decrypt sig with public key
  unsigned char *decrypt = new unsigned char[csize];
  if (!rsa.decrypt(cipher, decrypt))
  {
    serr << "Can't decrypt licence signature\n";
    return false;
  }

  // Compare decrypted signature with our digest
  bool ok = !memcmp(digest, decrypt, Crypto::SHA1::DIGEST_LENGTH);

  delete[] cipher;
  delete[] decrypt;

  if (!ok)
  {
    serr << "Bad signature on licence\n";
    return false;
  }

  // Now look for window
  Time::Stamp now = Time::Stamp::now();
  string w_begin_s = get_value("window/@begin");
  if (w_begin_s.size())
  {
    Time::Stamp begin(w_begin_s);
    if (now < begin)
    {
      serr << "Licence has not yet begun\n";
      return false;
    }
  }

  string w_end_s   = get_value("window/@end");
  if (w_end_s.size())
  {
    Time::Stamp end(w_end_s);
    if (now >= end)
    {
      serr << "Licence has ended\n";
      return false;
    }
  }

  // OK
  return true;
}

//--------------------------------------------------------------------------
// Check usability of licence file on current machine
// Also validates signature
bool File::check()
{
  if (!validate()) return false;

  // Get our MAC addresses
  Net::UDPSocket socket;
  set<string> macs = socket.get_host_macs();

  // Get all <computer> elements
  list<XML::Element *> computers = get_elements("computers/computer");
  for(XML::ElementIterator cp(computers); cp; ++cp)
  {
    const XML::Element& ce = *cp;
    string allowed_mac = ce["mac"];
    allowed_mac = Text::canonicalise_space(allowed_mac);
    allowed_mac = Text::toupper(allowed_mac);
    if (macs.find(allowed_mac) != macs.end() || allowed_mac == "ANY")
      return true;
  }

  serr << "Licence not valid for this computer - this computer's MACs are:\n";
  for(set<string>::iterator p = macs.begin(); p!=macs.end(); ++p)
    serr << "  " << *p << endl;

  return false;
}

//--------------------------------------------------------------------------
// Get XML element for given named component (e.g. pumpd)
// Pointer to element, or 0 if not found.  Element is owned by File class
// and will be destroyed by it
XML::Element *File::get_component(const string& name)
{
  // Construct full XPath
  string xpath("components/");
  return get_element(xpath+name);
}

//--------------------------------------------------------------------------
// Get string for given description element
string File::get_description(const string& name)
{
  // Construct full XPath
  string xpath("description/");
  return get_value(xpath+name);
}

//--------------------------------------------------------------------------
// Sign an existing licence with private key (PEM format)
// Writes modified XML back to file
// Returns whether successful
bool File::sign(const string& private_key, const string& pass_phrase)
{
  XML::Element& root = get_root();
  if (!root)
  {
    serr << "Can't read licence file\n";
    return false;  // Failed to read
  }

  // Set initial signature to empty
  XML::Element& sig_e = root.make_child("signature");
  sig_e.content.clear();

  // Export XML as text
  string text = root.to_string();

  string encoded_sig;

  // Get SHA1
  Crypto::SHA1 sha1;
  sha1.update(text.c_str(), text.size());
  unsigned char digest[Crypto::SHA1::DIGEST_LENGTH];
  sha1.get_result(digest);

  // Get RSA private key
  Crypto::RSA rsa(true);
  rsa.key.set(private_key, true, pass_phrase);
  if (!rsa.key.valid)
  {
    serr << "Bad private key\n";
    return false;
  }

  // Encrypt digest with private key
  int csize = rsa.cipher_size();
  unsigned char *cipher = new unsigned char[csize];
  if (!rsa.encrypt(digest, Crypto::SHA1::DIGEST_LENGTH, cipher))
  {
    serr << "Can't encrypt digest\n";
    return false;
  }

  Text::Base64 base64;
  encoded_sig = base64.encode(cipher, csize, 72, "\n    ");

  // Replace signature with this - whitespace to make it look nice
  sig_e.content = "\n    ";
  sig_e.content += encoded_sig;
  sig_e.content += "\n  ";

  // Write it back
  return write();
}

}} // namespaces




