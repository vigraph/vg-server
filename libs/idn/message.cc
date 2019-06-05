//==========================================================================
// ViGraph IDN stream library: message.cc
//
// IDN Stream message implementation
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-idn.h"

namespace ViGraph { namespace IDN {

// Dump to given stream
void Message::dump(ostream& out)
{
  out << "Message channel " << (int)channel_id
      << " @" << timestamp << "+" << duration
      << " scm " << (int)scm
      << (is_last_fragment()?" last":"")
      << (once?" once":"")
      << (has_data?" data":"")
      << ": ";
  switch (chunk_type)
  {
    case Message::ChunkType::void_:
      out << "void";
      break;

    case Message::ChunkType::laser_wave_samples:
      out << "laser wave samples";
      break;

    case Message::ChunkType::laser_frame_samples_entire:
      out << "laser wave samples entire";
      break;

    case Message::ChunkType::laser_frame_samples_first:
      out << "laser frame samples first";
      break;

    case Message::ChunkType::octet_segment:
      out << "octet segment";
      break;

    case Message::ChunkType::octet_string:
      out << "octet string";
      break;

    case Message::ChunkType::dimmer_levels:
      out << "dimmer levels";
      break;

    case Message::ChunkType::laser_frame_samples_sequel:
      out << "laser frame samples sequel";
      break;
  }

  cout << endl;

  if (has_channel_configuration())
  {
    out << "Config SDM " << (int)config.sdm
        << (config.close?" close":"")
        << (config.routing?" routing":"")
        << " service ID " << (int)config.service_id
        << " mode ";
    switch (config.service_mode)
    {
      case Message::Config::ServiceMode::void_:
        out << "void";
        break;

      case Message::Config::ServiceMode::graphic_continuous:
        out << "graphic continuous";
        break;

      case Message::Config::ServiceMode::graphic_discrete:
        out << "graphic discrete";
        break;

      case Message::Config::ServiceMode::effects_continuous:
        out << "effects continuous";
        break;

      case Message::Config::ServiceMode::effects_discrete:
        out << "effects_discrete";
        break;

      case Message::Config::ServiceMode::dmx512_continuous:
        out << "DMX512 continuous";
        break;

      case Message::Config::ServiceMode::dmx512_discrete:
        out << "DMX512 discrete";
        break;
    }

    cout << endl;

    if (!config.tags.empty())
    {
      cout << "Tags:";
      for(auto tag: config.tags)
        cout << " (" << (int)tag.category << "," << (int)tag.subcategory
             << "," << (int)tag.identifier << "," << (int)tag.parameter << ")";
      cout << endl;
    }
  }
}


}} // namespaces
