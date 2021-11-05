//==========================================================================
// ViGraph Nexus queue: test-queue.cc
//
// Tests for queue library
//
// Copyright (c) 2021 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-nexus-queue.h"
#include <gtest/gtest.h>

namespace {

using namespace ViGraph;
  using namespace ViGraph::Nexus;
using namespace std;

TEST(QueueTest, empty_queue_is_empty)
{
  Queue q(Time::Duration(5));
  Time::Stamp now;
  EXPECT_TRUE(q.get_waiting().empty());
  EXPECT_EQ("", q.get_active());
  EXPECT_EQ(0, q.get_time_remaining(now));
  EXPECT_EQ("", q.check_time_up(now));
}

TEST(QueueTest, first_id_becomes_current)
{
  Queue q(Time::Duration(5));
  Time::Stamp now;
  q.add("foo", now);
  EXPECT_EQ("foo", q.get_active());
  EXPECT_EQ(5, q.get_time_remaining(now));
  EXPECT_TRUE(q.get_waiting().empty());
  EXPECT_EQ("foo", q.check_time_up(now));
}

TEST(QueueTest, second_id_is_queued)
{
  Queue q(Time::Duration(5));
  Time::Stamp now;
  q.add("foo", now);
  q.add("bar", now);
  EXPECT_EQ("foo", q.get_active());
  EXPECT_EQ(5, q.get_time_remaining(now));
  const auto& w = q.get_waiting();
  ASSERT_EQ(1, w.size());
  ASSERT_EQ("bar", w.front());
  EXPECT_EQ("foo", q.check_time_up(now));
}

TEST(QueueTest, ids_are_timed_out_and_removed)
{
  Queue q(Time::Duration(5));
  Time::Stamp now;

  q.add("foo", now);
  q.add("bar", now);
  EXPECT_EQ("foo", q.get_active());
  EXPECT_EQ("foo", q.check_time_up(now));

  Time::Stamp now_plus_3(now+Time::Duration(3));
  EXPECT_EQ(2, q.get_time_remaining(now_plus_3));
  EXPECT_EQ("foo", q.check_time_up(now_plus_3));

  Time::Stamp now_plus_5(now+Time::Duration(5));
  EXPECT_EQ(0, q.get_time_remaining(now_plus_5));
  EXPECT_EQ("bar", q.check_time_up(now_plus_5));
  EXPECT_EQ(5, q.get_time_remaining(now_plus_5));

  Time::Stamp now_plus_10(now+Time::Duration(10));
  EXPECT_EQ(0, q.get_time_remaining(now_plus_10));
  EXPECT_EQ("", q.check_time_up(now_plus_10));
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
