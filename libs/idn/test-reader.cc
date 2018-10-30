//==========================================================================
// ViGraph IDN stream library: test-reader.cc
//
// Tests for IDN stream reader
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-idn.h"
#include <gtest/gtest.h>

namespace {

using namespace ViGraph;
using namespace ViGraph::Geometry;
using namespace ViGraph::IDN;

TEST(IDNReaderTest, TestReadEmptyFileThrows)
{
  string data = "";
  Channel::StringReader sr(data);
  Reader reader(sr);
  Message message;
  ASSERT_THROW(reader.read(message), runtime_error);
}

TEST(IDNReaderTest, TestMissingCNLTopBitThrows)
{
  string data("\x00\x08\x2a\x00"         // No CNL top bit
              "\x00\x00\x00\x00", 8);    // timestamp
  Channel::StringReader sr(data);
  Reader reader(sr);
  Message message;
  ASSERT_THROW(reader.read(message), runtime_error);
}

TEST(IDNReaderTest, TestBasicMessageHeader)
{
  string data("\x00\x08\xaa\x01"         // channel ID 42 (0x2a), type 1
              "\xaa\xbb\xcc\xdd", 8);    // timestamp

  Channel::StringReader sr(data);
  Reader reader(sr);
  Message message;
  ASSERT_NO_THROW(reader.read(message));
  EXPECT_EQ(false, message.cclf);
  EXPECT_EQ(42, message.channel_id);
  EXPECT_EQ(IDN::Message::ChunkType::laser_wave_samples, message.chunk_type);
  EXPECT_EQ(0xaabbccdd, message.timestamp);
}

TEST(IDNReaderTest, TestShortMessageWithCCLFThrows)
{
  string data("\x00\x08\xc0\x00"         // No CNL top bit
              "\x00\x00\x00\x00", 8);    // timestamp
  Channel::StringReader sr(data);
  Reader reader(sr);
  Message message;
  ASSERT_THROW(reader.read(message), runtime_error);
}

TEST(IDNReaderTest, TestMessageWithConfig)
{
  string data("\x00\x0c\xc0\x01"         // cclf, id 0, type 1
              "\xaa\xbb\xcc\xdd"         // timestamp
              "\x00\x23\x2a\x01", 12);   // sdm=2, close, routing
                                         // service ID 42,
                                         // mode = graphic-continuous

  Channel::StringReader sr(data);
  Reader reader(sr);
  Message message;
  ASSERT_NO_THROW(reader.read(message));
  EXPECT_TRUE(message.has_channel_configuration());
  EXPECT_EQ(0, message.channel_id);
  EXPECT_EQ(IDN::Message::ChunkType::laser_wave_samples, message.chunk_type);
  EXPECT_EQ(0xaabbccdd, message.timestamp);
  EXPECT_EQ(2, message.config.sdm);
  EXPECT_EQ(42, message.config.service_id);
  EXPECT_EQ(Message::Config::ServiceMode::graphic_continuous,
            message.config.service_mode);
  EXPECT_TRUE(message.get_routing());
  EXPECT_TRUE(message.get_close());
}

TEST(IDNReaderTest, TestMessageWithConfigAndTags)
{
  // Note SCWC=1 32-bit word = 2 tags
  string data("\x00\x10\xc0\x01"         // cclf, id 0, type 1
              "\xaa\xbb\xcc\xdd"         // timestamp
              "\x01\x00\x00\x01"         // mode = graphic-continuous
              "\x01\x23"                 // Tag 0:1 (2,3)
              "\x52\x7E"                 // Tag red, 638nm
              , 16);

  Channel::StringReader sr(data);
  Reader reader(sr);
  Message message;
  ASSERT_NO_THROW(reader.read(message));
  ASSERT_TRUE(message.has_channel_configuration());
  EXPECT_EQ(Message::Config::ServiceMode::graphic_continuous,
            message.config.service_mode);
  ASSERT_EQ(2, message.config.tags.size());
  EXPECT_EQ(0x0123, message.config.tags[0].to_word());
  EXPECT_EQ(0x527E, message.config.tags[1].to_word());
}

TEST(IDNReaderTest, TestMessageWithData)
{
  string data("\x00\x10\x80\x01"         // no cclf, ID 0
              "\xaa\xbb\xcc\xdd"         // timestamp
              "\x31\x45\x67\x89"         // SCM 3, once, dur 0x456789
              "\x01\xff\x12\x34"         // data
              "\xDE\xAD"                 // Note cruft that should not be read
              , 18);

  Channel::StringReader sr(data);
  Reader reader(sr);
  Message message;
  ASSERT_NO_THROW(reader.read(message));
  ASSERT_FALSE(message.has_channel_configuration());
  EXPECT_EQ(3, message.scm);
  EXPECT_TRUE(message.once);
  EXPECT_EQ(0x456789, message.duration);
  ASSERT_EQ(4, message.data.size());
  EXPECT_EQ(0x01, message.data[0]);
  EXPECT_EQ(0xFF, message.data[1]);
  EXPECT_EQ(0x12, message.data[2]);
  EXPECT_EQ(0x34, message.data[3]);
  EXPECT_EQ(16, reader.get_offset());  // Make sure it has left the cruft
}

TEST(IDNReaderTest, TestSequelMessageWithDataNeedsNoDataHeader)
{
  string data("\x00\x0c\x80\xc0"         // no cclf, ID 0
              "\xaa\xbb\xcc\xdd"         // timestamp
              "\x01\xff\x12\x34"         // data
              "\xDE\xAD"                 // Note cruft that should not be read
              , 14);

  Channel::StringReader sr(data);
  Reader reader(sr);
  Message message;
  ASSERT_NO_THROW(reader.read(message));
  ASSERT_FALSE(message.has_channel_configuration());
  ASSERT_EQ(4, message.data.size());
  EXPECT_EQ(0x01, message.data[0]);
  EXPECT_EQ(0xFF, message.data[1]);
  EXPECT_EQ(0x12, message.data[2]);
  EXPECT_EQ(0x34, message.data[3]);
  EXPECT_EQ(12, reader.get_offset());  // Make sure it has left the cruft
}

TEST(IDNReaderTest, TestMessageWithConfigAndTagsAndData)
{
  // Note SCWC = 2 32-bit words = 4 tags
  string data("\x00\x1c\xc0\x01"         // cclf, id 0, type 1
              "\xaa\xbb\xcc\xdd"         // timestamp
              "\x02\x00\x00\x01"         // mode = graphic-continuous
              "\x01\x23"                 // Tag 0:1 (2,3)
              "\x42\x00"                 // Tag x
              "\x52\x7E"                 // Tag red, 638nm
              "\x00\x00"                 // Void tag
              "\x00\x45\x67\x89"         // SCM 0, duration 0x456789
              "\x01\xff\x12\x34"         // Data
              "\xDE\xAD"                 // Note cruft that should not be read
              , 30);

  Channel::StringReader sr(data);
  Reader reader(sr);
  Message message;
  ASSERT_NO_THROW(reader.read(message));
  ASSERT_TRUE(message.has_channel_configuration());
  EXPECT_EQ(Message::Config::ServiceMode::graphic_continuous,
            message.config.service_mode);
  ASSERT_EQ(3, message.config.tags.size());
  EXPECT_EQ(0x0123, message.config.tags[0].to_word());
  EXPECT_EQ(Tags::x, message.config.tags[1]);
  EXPECT_EQ(Tags::red, message.config.tags[2]);
  EXPECT_EQ(0, message.scm);
  EXPECT_FALSE(message.once);
  EXPECT_EQ(0x456789, message.duration);
  ASSERT_EQ(4, message.data.size());
  EXPECT_EQ(0x01, message.data[0]);
  EXPECT_EQ(0xFF, message.data[1]);
  EXPECT_EQ(0x12, message.data[2]);
  EXPECT_EQ(0x34, message.data[3]);
  EXPECT_EQ(28, reader.get_offset());  // Make sure it has left the cruft
}

TEST(IDNReaderTest, TestMessageVoidTagsSkipData)
{
  // Note SCWC = 2 32-bit words = 4 tags
  string data("\x00\x14\xc0\x01"         // cclf, id 0, type 1
              "\xaa\xbb\xcc\xdd"         // timestamp
              "\x02\x00\x00\x01"         // mode = graphic-continuous
              "\x00\x02"                 // Void, skip 2
              "\xDE\xAD"                 // Junk to skip
              "\xDE\xAD"
              "\x52\x7E"                 // Tag red, 638nm
              , 20);

  Channel::StringReader sr(data);
  Reader reader(sr);
  Message message;
  ASSERT_NO_THROW(reader.read(message));
  ASSERT_TRUE(message.has_channel_configuration());
  EXPECT_EQ(Message::Config::ServiceMode::graphic_continuous,
            message.config.service_mode);
  ASSERT_EQ(1, message.config.tags.size());
  EXPECT_EQ(Tags::red, message.config.tags[0]);
}

TEST(IDNReaderTest, TestHelloHeader)
{
  string data("\x40\x2a\x12\x34"    // message, flags 42, seq 1234
              , 4);

  Channel::StringReader sr(data);
  Reader reader(sr);
  HelloHeader hello;
  ASSERT_NO_THROW(reader.read(hello));
  EXPECT_EQ(hello.command, HelloHeader::Command::message);
  EXPECT_EQ(42, hello.flags);
  EXPECT_EQ(0x1234, hello.sequence);
  EXPECT_EQ(4, hello.length());
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
