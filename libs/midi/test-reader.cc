//==========================================================================
// ViGraph MIDI library: test-reader.cc
//
// Tests for MIDI format reader
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-midi.h"
#include <gtest/gtest.h>

namespace {

using namespace ViGraph;
using namespace ViGraph::MIDI;

TEST(MIDIReaderTest, TestReadEmptyBufferGivesNone)
{
  Reader reader;
  Event event = reader.read();
  ASSERT_EQ(Event::Type::none, event.type);
}

TEST(MIDIReaderTest, TestReadNoteOff)
{
  Reader reader;
  reader.add(0x87);
  reader.add(33);
  reader.add(55);
  Event event = reader.read();
  ASSERT_EQ(Event::Type::note_off, event.type);
  ASSERT_EQ(8, event.channel);
  ASSERT_EQ(33, event.key);
  ASSERT_EQ(55, event.value);
}

TEST(MIDIReaderTest, TestReadNoteOn)
{
  Reader reader;
  reader.add(0x97);
  reader.add(33);
  reader.add(55);
  Event event = reader.read();
  ASSERT_EQ(Event::Type::note_on, event.type);
  ASSERT_EQ(8, event.channel);
  ASSERT_EQ(33, event.key);
  ASSERT_EQ(55, event.value);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
