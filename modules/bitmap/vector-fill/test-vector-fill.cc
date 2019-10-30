//==========================================================================
// ViGraph dataflow module: bitmap/vector-fill/test-vector-fill.cc
//
// Tests for <translate> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector/vector-module.h"
#include "../bitmap-module.h"
#include "../../module-test.h"

class TranslateTest: public GraphTester
{
public:
  TranslateTest()
  {
    loader.load("./vg-module-bitmap-vector-fill.so");
  }
};

//const auto sample_rate = 1;

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
