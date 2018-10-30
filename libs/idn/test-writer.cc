//==========================================================================
// ViGraph IDN stream library: test-writer.cc
//
// Tests for IDN stream writer
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-idn.h"
#include <gtest/gtest.h>

namespace {

using namespace ViGraph;
using namespace ViGraph::IDN;

TEST(IDNWriterTest, TestBasicMessageHeader)
{
  string expected_data("\x00\x08\xaa\x01"       // channel ID 42 (0x2a), type 1
                       "\xaa\xbb\xcc\xdd", 8);  // timestamp

  string actual_data;
  Channel::StringWriter sw(actual_data);
  Writer writer(sw);
  Message message(42, Message::ChunkType::laser_wave_samples);
  message.timestamp = 0xaabbccdd;

  ASSERT_NO_THROW(writer.write(message));
  EXPECT_EQ(expected_data, actual_data);
}

TEST(IDNWriterTest, TestMessageWithConfiguration)
{
  string expected_data("\x00\x0c\xc0\x01"         // with cclf, ID 0
                       "\xaa\xbb\xcc\xdd"         // timestamp
                       "\x00\x23\x2a\x01", 12);   // sdm=2, close, routing
                                                  // service ID 42,
                                                  // mode = graphic-continuous

  string actual_data;
  Channel::StringWriter sw(actual_data);
  Writer writer(sw);
  Message message(0, Message::ChunkType::laser_wave_samples);
  message.timestamp = 0xaabbccdd;
  message.add_configuration(Message::Config::ServiceMode::graphic_continuous, 42);
  message.config.sdm = 2;
  message.set_routing();
  message.set_close();

  ASSERT_NO_THROW(writer.write(message));
  EXPECT_EQ(expected_data, actual_data);
}

TEST(IDNWriterTest, TestMessageWithConfigurationAndTags)
{
  // Note two tags = 1 32-bit word, SCWC = 1
  string expected_data("\x00\x10\xc0\x01"         // with cclf, ID 0
                       "\xaa\xbb\xcc\xdd"         // timestamp
                       "\x01\x00\x00\x01"         // mode = graphic-continuous
                       "\x01\x23"                 // Tag 0:1 (2,3)
                       "\x52\x7E"                 // Tag red, 638nm
                       , 16);
  string actual_data;
  Channel::StringWriter sw(actual_data);
  Writer writer(sw);
  Message message(0, Message::ChunkType::laser_wave_samples);
  message.timestamp = 0xaabbccdd;
  message.add_configuration(Message::Config::ServiceMode::graphic_continuous);
  message.add_tag(Tag(0,1,2,3));
  message.add_tag(Tag(0x527E));
  ASSERT_NO_THROW(writer.write(message));
  EXPECT_EQ(expected_data, actual_data);
}

TEST(IDNWriterTest, TestMessageWithData)
{
  string expected_data("\x00\x10\x80\x01"         // no cclf, ID 0
                       "\xaa\xbb\xcc\xdd"         // timestamp
                       "\x31\x45\x67\x89"         // SCM 3, once, dur 0x456789
                       "\x01\xff\x12\x34"         // data
                       , 16);
  string actual_data;
  Channel::StringWriter sw(actual_data);
  Writer writer(sw);
  Message message(0, Message::ChunkType::laser_wave_samples);
  message.timestamp = 0xaabbccdd;
  message.set_data_header(0x456789, true, 3);
  message.add_data(0x01);
  message.add_data(0xFF);
  message.add_data16(0x1234);
  ASSERT_NO_THROW(writer.write(message));
  EXPECT_EQ(expected_data, actual_data);
}

TEST(IDNWriterTest, TestSequelMessageWithDataHasNoDataHeader)
{
  string expected_data("\x00\x0c\x80\xc0"         // no cclf, ID 0
                       "\xaa\xbb\xcc\xdd"         // timestamp
                       "\x01\xff\x12\x34"         // data
                       , 12);
  string actual_data;
  Channel::StringWriter sw(actual_data);
  Writer writer(sw);
  Message message(0, Message::ChunkType::laser_frame_samples_sequel);
  message.timestamp = 0xaabbccdd;
  message.set_data_header(0x456789, true, 3);  // But ignored
  message.add_data(0x01);
  message.add_data(0xFF);
  message.add_data16(0x1234);
  ASSERT_NO_THROW(writer.write(message));
  EXPECT_EQ(expected_data, actual_data);
}

TEST(IDNWriterTest, TestMessageWithConfigurationAndTagsAndData)
{
  // Note 3 tags = 2 32-bit words with padding, SCWC = 2
  string expected_data("\x00\x1c\xc0\x01"         // with cclf, ID 0
                       "\xaa\xbb\xcc\xdd"         // timestamp
                       "\x02\x00\x00\x01"         // mode = graphic-continuous
                       "\x01\x23"                 // Tag 0:1 (2,3)
                       "\x42\x00"                 // Tag x
                       "\x52\x7E"                 // Tag red, 638nm
                       "\x00\x00"                 // Void tag for padding to 32
                       "\x00\x45\x67\x89"         // SCM 0, duration 0x456789
                       "\x01\xff\x12\x34"         // data
                       , 28);
  string actual_data;
  Channel::StringWriter sw(actual_data);
  Writer writer(sw);
  Message message(0, Message::ChunkType::laser_wave_samples);
  message.timestamp = 0xaabbccdd;
  message.add_configuration(Message::Config::ServiceMode::graphic_continuous);
  message.add_tag(Tag(0,1,2,3));
  message.add_tag(Tags::x);
  message.add_tag(Tags::red);
  message.set_data_header(0x456789);
  message.add_data(0x01);
  message.add_data(0xFF);
  message.add_data16(0x1234);
  ASSERT_NO_THROW(writer.write(message));
  EXPECT_EQ(expected_data, actual_data);
}

TEST(IDNWriterTest, TestHelloHeader)
{
  string expected_data("\x40\x2a\x12\x34"    // message, flags 42, seq 1234
                       , 4);

  string actual_data;
  Channel::StringWriter sw(actual_data);
  Writer writer(sw);
  HelloHeader hello(HelloHeader::Command::message, 0x1234);
  hello.flags = 42;
  EXPECT_EQ(4, hello.length());

  ASSERT_NO_THROW(writer.write(hello));
  EXPECT_EQ(expected_data, actual_data);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
