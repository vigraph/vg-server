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
  ASSERT_EQ(60, get_midi_number("C"));
  ASSERT_EQ(62, get_midi_number("D"));
  ASSERT_EQ(64, get_midi_number("e"));
  ASSERT_EQ(65, get_midi_number("F"));
  ASSERT_EQ(67, get_midi_number("  G"));
  ASSERT_EQ(69, get_midi_number("A"));
  ASSERT_EQ(71, get_midi_number("B"));
}

TEST(MIDINoteTest, TestVariousOctaves)
{
  ASSERT_EQ(24, get_midi_number("C1"));
  ASSERT_EQ(38, get_midi_number("D 2"));
  ASSERT_EQ(52, get_midi_number("E3"));
  ASSERT_EQ(65, get_midi_number("F4"));
  ASSERT_EQ(79, get_midi_number("g5"));
  ASSERT_EQ(93, get_midi_number("A6"));
  ASSERT_EQ(107, get_midi_number("B7"));
}

TEST(MIDINoteTest, TestSharps)
{
  ASSERT_EQ(49, get_midi_number("C#3"));
  ASSERT_EQ(82, get_midi_number("A# 5"));
  ASSERT_EQ(102, get_midi_number("f#7"));
}

TEST(MIDINoteTest, TestFlats)
{
  ASSERT_EQ(58, get_midi_number(" bb3"));
  ASSERT_EQ(80, get_midi_number("Ab5"));
  ASSERT_EQ(102, get_midi_number("Gb7 "));
}

TEST(MIDINoteTest, TestFrequency)
{
  ASSERT_NEAR(311.13, get_midi_frequency(63), 0.01);
  ASSERT_NEAR(3322.44, get_midi_frequency(104), 0.01);
  ASSERT_NEAR(7040.00, get_midi_frequency(117), 0.01);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
