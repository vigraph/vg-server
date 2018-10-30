//==========================================================================
// ViGraph dataflow module: vector/filters/test-collision-detect.cc
//
// Tests for <collision-detect> filter
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module-test.h"
ModuleLoader loader;

TEST(CollisionDetectTest, TestWithNoCollisionGroupFails)
{
  const string& xml = R"(
    <graph>
      <svg path='M0,0 L1,0'/>
      <collision-detect target='foo'/>
    </graph>
  )";

  BadFrameGenerator gen(xml, loader);
}

TEST(CollisionDetectTest, TestNoTargetsDoesNotNeedConnection)
{
  const string& xml = R"(
    <graph>
      <svg path='M0,0 L1,0'/>
      <collision-detect/>
    </graph>
  )";

  FrameGenerator gen(xml, loader, 0);
}

TEST(CollisionDetectTest, TestTargetSpecifiedButAbsentFails)
{
  const string& xml = R"(
    <graph>
      <svg path='M0,0 L1,0'/>
      <collision-detect target='foo'/>
    </graph>
  )";

  BadFrameGenerator gen(xml, loader);
}

TEST(CollisionDetectTest, TestConnectionWithTarget)
{
  const string& xml = R"(
    <graph>
      <svg path='M0,0 L1,0'/>
      <collision-detect target='m'/>
      <set id='m' property='x'/>
      <translate/>
    </graph>
  )";

  FrameGenerator gen(xml, loader, 0);
}

int main(int argc, char **argv)
{
  if (argc > 1 && string(argv[1]) == "-v")
  {
    auto chan_out = new Log::StreamChannel{&cout};
    Log::logger.connect(chan_out);
  }

  ::testing::InitGoogleTest(&argc, argv);
  loader.load("../../../core/controls/set/vg-module-core-control-set.so");
  loader.load("../../sources/svg/vg-module-vector-source-svg.so");
  loader.load("../../services/collision-detector/vg-module-vector-service-collision-detector.so");
  loader.load("../translate/vg-module-vector-filter-translate.so");
  loader.load("./vg-module-vector-filter-collision-detect.so");
  loader.configure_with_service("collision-detector");
  return RUN_ALL_TESTS();
}
