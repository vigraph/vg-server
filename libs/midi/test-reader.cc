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
  Event event = reader.get();
  ASSERT_EQ(Event::Type::none, event.type);
}

TEST(MIDIReaderTest, TestReadNoteOff)
{
  Reader reader;
  reader.add(0x87);
  reader.add(33);
  reader.add(55);
  Event event = reader.get();
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
  Event event = reader.get();
  ASSERT_EQ(Event::Type::note_on, event.type);
  ASSERT_EQ(8, event.channel);
  ASSERT_EQ(33, event.key);
  ASSERT_EQ(55, event.value);
}

TEST(MIDIReaderTest, TestReadControlChange)
{
  Reader reader;
  reader.add(0xBF);
  reader.add(1);
  reader.add(127);
  Event event = reader.get();
  ASSERT_EQ(Event::Type::control_change, event.type);
  ASSERT_EQ(16, event.channel);
  ASSERT_EQ(1, event.key);
  ASSERT_EQ(127, event.value);
}

TEST(MIDIReaderTest, TestReadTwoNoteOnsWithRunningStatus)
{
  Reader reader;
  reader.add(0x97);
  reader.add(33);
  reader.add(55);
  reader.add(34);
  reader.add(56);
  Event event = reader.get();
  ASSERT_EQ(Event::Type::note_on, event.type);
  ASSERT_EQ(8, event.channel);
  ASSERT_EQ(33, event.key);
  ASSERT_EQ(55, event.value);
  event = reader.get();
  ASSERT_EQ(Event::Type::note_on, event.type);
  ASSERT_EQ(8, event.channel);
  ASSERT_EQ(34, event.key);
  ASSERT_EQ(56, event.value);
}

TEST(MIDIReaderTest, TestReadNoteOnAfterJunk)
{
  Reader reader;
  reader.add(0x00);
  reader.add(0x01);
  reader.add(0x97);
  reader.add(33);
  reader.add(55);
  Event event = reader.get();
  ASSERT_EQ(Event::Type::note_on, event.type);
  ASSERT_EQ(8, event.channel);
  ASSERT_EQ(33, event.key);
  ASSERT_EQ(55, event.value);
}

TEST(MIDIReaderTest, TestReadNoteOnAfterUnrecognisedMessage)
{
  Reader reader;
  reader.add(0xf4);
  reader.add(0x97);
  reader.add(33);
  reader.add(55);
  Event event = reader.get();
  ASSERT_EQ(Event::Type::note_on, event.type);
  ASSERT_EQ(8, event.channel);
  ASSERT_EQ(33, event.key);
  ASSERT_EQ(55, event.value);
}

TEST(MIDIReaderTest, TestReadNoteOnWithInterspersedRealTimes)
{
  Reader reader;
  reader.add(0xf8);
  reader.add(0x97);
  reader.add(0xf9);
  reader.add(33);
  reader.add(0xfe);
  reader.add(55);
  reader.add(0xff);
  Event event = reader.get();
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
