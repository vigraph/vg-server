//==========================================================================
// ViGraph MIDI library: test-note.cc
//
// Tests for MIDI note functions
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-midi.h"
#include <gtest/gtest.h>

namespace {

using namespace ViGraph;
using namespace ViGraph::MIDI;

TEST(MIDINoteTest, TestBaseOctave)
{
  ASSERT_EQ(60, get_midi_note("C"));
  ASSERT_EQ(62, get_midi_note("D"));
  ASSERT_EQ(64, get_midi_note("e"));
  ASSERT_EQ(65, get_midi_note("F"));
  ASSERT_EQ(67, get_midi_note("  G"));
  ASSERT_EQ(69, get_midi_note("A"));
  ASSERT_EQ(71, get_midi_note("B"));
}

TEST(MIDINoteTest, TestVariousOctaves)
{
  ASSERT_EQ(24, get_midi_note("C1"));
  ASSERT_EQ(38, get_midi_note("D 2"));
  ASSERT_EQ(52, get_midi_note("E3"));
  ASSERT_EQ(65, get_midi_note("F4"));
  ASSERT_EQ(79, get_midi_note("g5"));
  ASSERT_EQ(93, get_midi_note("A6"));
  ASSERT_EQ(107, get_midi_note("B7"));
}

TEST(MIDINoteTest, TestSharps)
{
  ASSERT_EQ(49, get_midi_note("C#3"));
  ASSERT_EQ(82, get_midi_note("A# 5"));
  ASSERT_EQ(102, get_midi_note("f#7"));
}

TEST(MIDINoteTest, TestFlats)
{
  ASSERT_EQ(58, get_midi_note(" bb3"));
  ASSERT_EQ(80, get_midi_note("Ab5"));
  ASSERT_EQ(102, get_midi_note("Gb7 "));
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
