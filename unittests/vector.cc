
#include <gtest/gtest.h>

#include "vector.h"

namespace {

TEST(vector, basic)
{
  vector<3> v;
  EXPECT_EQ(v.to_string(), "(0,0,0)");
  Vector3d vv(1, 2, 3);
  EXPECT_EQ(vv.to_string(), "(1,2,3)");
}

}
