// Copyright 2014 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include <gtest/gtest.h>

#include "Common/CommonFuncs.h"

TEST(CommonFuncs, CrashMacro)
{
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH({ Crash(); }, "");
#endif
}
