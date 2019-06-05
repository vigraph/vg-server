//==========================================================================
// ViGraph vector graphics: vg-idn.h
//
// Reader for the ILDA Digital Network standard
// http://www.ilda.com/resources/StandardsDocs/ILDA_IDN-Stream_rev001.pdf
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VG_IDN_H
#define __VG_IDN_H

#include <vector>
#include <string>
#include <istream>
#include "vg-geometry.h"
#include "ot-chan.h"

namespace ViGraph { namespace IDN {

// Make our lives easier without polluting anyone else
using namespace std;
using namespace ViGraph;
using namespace ObTools;
using namespace ViGraph::Geometry;

//==========================================================================
// IDN Stream Configuration Tag (metadata)
struct Tag
{
  uint8_t category{0};
  uint8_t subcategory{0};
  uint8_t identifier{0};
  uint8_t parameter{0};

  // Constructors
  Tag() {}
  Tag(uint8_t _cat, uint8_t _subcat, uint8_t _id=0, uint8_t _prm=0):
    category(_cat), subcategory(_subcat), identifier(_id), parameter(_prm) {}

  // Construct from uint16_t word
  Tag(uint16_t word);

  // Convert to uint16_t
  uint16_t to_word() const;

  // Compare
  bool operator==(const Tag& t) const
  { return category == t.category && subcategory == t.subcategory
    && identifier == t.identifier && parameter == t.parameter; }

  // Create a colour tag from 10-bit wavelength
  static Tag colour(uint16_t wavelength)
  { return Tag(5, wavelength >> 8, (wavelength >> 4)&0x0f, wavelength&0x0f); }
};

// Standard Tags
namespace Tags
{
  const Tag x{4,2,0,0};
  const Tag y{4,2,1,0};
  const Tag z{4,2,2,0};
  const Tag red{Tag::colour(638)};
  const Tag green{Tag::colour(532)};
  const Tag blue{Tag::colour(460)};
  const Tag prec16{4,0,1,0};
  const Tag intensity{5,12,1,0};
}

//==========================================================================
// IDN Stream Channel Message
struct Message
{
  // length is calculated from content
  bool cclf{false};
  uint8_t channel_id{0};

  enum class ChunkType
  {
    void_                       = 0x00,
    laser_wave_samples          = 0x01,
    laser_frame_samples_entire  = 0x02,
    laser_frame_samples_first   = 0x03,

    octet_segment               = 0x10,
    octet_string                = 0x11,

    dimmer_levels               = 0x18,

    laser_frame_samples_sequel  = 0xc0
  } chunk_type{ChunkType::void_};

  uint32_t timestamp{0};   // Microseconds

  // optional Channel Configuration Header
  struct Config
  {
    // SCWC calculated from tag list
    uint8_t sdm{0};  // Service data match
    bool close{false};
    bool routing{false};
    uint8_t service_id{0};
    enum class ServiceMode
    {
      void_              = 0x00,
      graphic_continuous = 0x01,
      graphic_discrete   = 0x02,
      effects_continuous = 0x03,
      effects_discrete   = 0x04,
      dmx512_continuous  = 0x05,
      dmx512_discrete    = 0x06
    } service_mode{ServiceMode::void_};

    vector<Tag> tags;
  } config;

  uint8_t scm{0};
  bool once{false};
  uint32_t duration{0}; // microseconds
  bool has_data{false}; // even if empty
  vector<uint8_t> data;

  //-----------------------------------------------------------------------
  // Constructors
  Message() {}
  Message(ChunkType _chunk_type): chunk_type(_chunk_type) {}
  Message(uint8_t _channel_id, ChunkType _chunk_type):
    channel_id(_channel_id), chunk_type(_chunk_type) {}

  // Get the byte length required for the message
  size_t length()
  {
    size_t length = 8;
    if (has_channel_configuration())
      length += 4 + 4*((config.tags.size()+1)/2); // Rounded up to 32-bit words
    if (has_data && !is_sequel())
      length += 4;  // Data chunk header
    length += data.size();
    return length;
  }

  //-----------------------------------------------------------------------
  // Check if it's a sequel
  bool is_sequel() const
  { return chunk_type >= ChunkType::laser_frame_samples_sequel; }

  //-----------------------------------------------------------------------
  // Unpacking cclf - meaning depends on chunk type
  bool has_channel_configuration() const
  { return cclf && !is_sequel(); }

  bool is_last_fragment() const
  { return cclf && is_sequel(); }

  //-----------------------------------------------------------------------
  // Add configuration
  void add_configuration(Config::ServiceMode mode, uint8_t _service_id = 0)
  { cclf = true; config.service_mode = mode; config.service_id = _service_id; }

  //-----------------------------------------------------------------------
  // Get/Set configuration flags
  bool get_routing() const
  { return has_channel_configuration() && config.routing; }
  void set_routing() { config.routing = true; }
  bool get_close() const
  { return has_channel_configuration() && config.close; }
  void set_close()   { config.close   = true; }

  //-----------------------------------------------------------------------
  // Add a tag
  void add_tag(const Tag& tag) { config.tags.push_back(tag); }

  //-----------------------------------------------------------------------
  // Set the data header, duration in microseconds
  void set_data_header(uint32_t _duration, bool _once = false, uint8_t _scm = 0)
  { duration = _duration; once = _once; scm = _scm; has_data = true; }

  //-----------------------------------------------------------------------
  // Add a data byte
  void add_data(uint8_t byte) { data.push_back(byte); }

  //-----------------------------------------------------------------------
  // Add a 16-bit data word
  void add_data16(uint16_t word)
  { data.push_back(word >> 8); data.push_back(word & 0xff); }

  // Dump to given stream
  void dump(ostream& out);
};

//==========================================================================
// IDN Hello message header
struct HelloHeader
{
  uint8_t flags{0};

  enum class Command
  {
    void_         = 0x00,
    ping_request  = 0x08,
    ping_reply    = 0x09,
    scan_request  = 0x10,
    scan_reply    = 0x11,
    message       = 0x40
  } command{Command::void_};

  uint16_t sequence{0};

  // Constructors
  HelloHeader() {}
  HelloHeader(Command _cmd, uint16_t _seq=0): command(_cmd), sequence(_seq) {}

  // Get length
  size_t length() { return 4; }

  // Dump to given stream
  void dump(ostream& out);
};

//==========================================================================
// Hello protocol
namespace Hello
{
  const int default_port = 7255;
}

//==========================================================================
// IDN Stream format reader
class Reader
{
  Channel::Reader& input;

 public:
  //-----------------------------------------------------------------------
  // Constructor on a channel reader
  Reader(Channel::Reader& in): input(in) {}

  //-----------------------------------------------------------------------
  // Read a single IDN Stream Channel Message
  // Throws runtime_error if it fails
  void read(Message& message);

  //-----------------------------------------------------------------------
  // Read an IDN Hello header
  // Throws runtime_error if it fails
  void read(HelloHeader& hello);

  //-----------------------------------------------------------------------
  // Get the input offset (for testing)
  uint16_t get_offset() { return input.get_offset(); }
};

//==========================================================================
// IDN Stream format writer
class Writer
{
  Channel::Writer& output;

 public:
  //-----------------------------------------------------------------------
  // Constructor on a channel writer
  Writer(Channel::Writer& out): output(out) {}

  //-----------------------------------------------------------------------
  // Write a single IDN Stream Channel Message
  // Throws runtime_error if it fails
  void write(Message& message);

  //-----------------------------------------------------------------------
  // Write an IDN-Hello header
  // Throws runtime_error if it fails
  void write(HelloHeader& hello);

};

//==========================================================================
}} //namespaces
#endif // !__VG_IDN_H
