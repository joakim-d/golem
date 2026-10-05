#include "golem/version.h"

#include <gtest/gtest.h>

TEST(
    Version,
    IsNotEmpty)
{
    EXPECT_FALSE(golem::version().empty());
}
