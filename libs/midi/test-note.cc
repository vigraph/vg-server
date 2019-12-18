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
  EXPECT_EQ(60, get_midi_number("C"));
  EXPECT_EQ(62, get_midi_number("D"));
  EXPECT_EQ(64, get_midi_number("e"));
  EXPECT_EQ(65, get_midi_number("F"));
  EXPECT_EQ(67, get_midi_number("  G"));
  EXPECT_EQ(69, get_midi_number("A"));
  EXPECT_EQ(71, get_midi_number("B"));
}

TEST(MIDINoteTest, TestVariousOctaves)
{
  EXPECT_EQ(24, get_midi_number("C1"));
  EXPECT_EQ(38, get_midi_number("D 2"));
  EXPECT_EQ(52, get_midi_number("E3"));
  EXPECT_EQ(65, get_midi_number("F4"));
  EXPECT_EQ(79, get_midi_number("g5"));
  EXPECT_EQ(93, get_midi_number("A6"));
  EXPECT_EQ(107, get_midi_number("B7"));
}

TEST(MIDINoteTest, TestSharps)
{
  EXPECT_EQ(49, get_midi_number("C#3"));
  EXPECT_EQ(82, get_midi_number("A# 5"));
  EXPECT_EQ(102, get_midi_number("f#7"));
}

TEST(MIDINoteTest, TestFlats)
{
  EXPECT_EQ(58, get_midi_number(" bb3"));
  EXPECT_EQ(80, get_midi_number("Ab5"));
  EXPECT_EQ(102, get_midi_number("Gb7 "));
}

TEST(MIDINoteTest, TestNoteToCV)
{
  EXPECT_EQ(0, get_midi_cv(60));
  EXPECT_EQ(1, get_midi_cv(72));
  EXPECT_EQ(-1, get_midi_cv(48));
  EXPECT_DOUBLE_EQ(0.5, get_midi_cv(66));
}

TEST(MIDINoteTest, TestNumberToNote)
{
  EXPECT_EQ("A#2", get_midi_note(46));
  EXPECT_EQ("C3", get_midi_note(48));
  EXPECT_EQ("G9", get_midi_note(127));
  EXPECT_EQ("A0", get_midi_note(21));
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
