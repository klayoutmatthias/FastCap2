
#include <gtest/gtest.h>

#include "vector.h"

namespace {

TEST(vector, basic)
{
  vector<3> v;
  EXPECT_EQ(v.to_string(), "(0,0,0)");
  Vector3d vv(1, 2, 3);
  EXPECT_EQ(vv.to_string(), "(1,2,3)");
  EXPECT_EQ(Vector3d(vv).to_string(), "(1,2,3)");
  EXPECT_EQ(Vector3d(vv.z(), vv.y(), vv.x()).to_string(), "(3,2,1)");
  EXPECT_EQ(Vector3d(vv[2], vv[1], vv[0]).to_string(), "(3,2,1)");
}

TEST(vector, arith)
{
  Vector3d vv1(1, 2, 3), vv2(10, 20, 30);
  Vector3d x;
  EXPECT_EQ((vv1 + vv2).to_string(), "(11,22,33)");
  x = vv1;
  x += vv2;
  EXPECT_EQ(x.to_string(), "(11,22,33)");
  EXPECT_EQ((vv1 - vv2).to_string(), "(-9,-18,-27)");
  x = vv1;
  x -= vv2;
  EXPECT_EQ(x.to_string(), "(-9,-18,-27)");
  EXPECT_EQ((-vv1).to_string(), "(-1,-2,-3)");
  EXPECT_EQ((vv1 * 2.0).to_string(), "(2,4,6)");
  EXPECT_EQ((2.0 * vv1).to_string(), "(2,4,6)");
  x = vv1;
  x *= 2.0;
  EXPECT_EQ(x.to_string(), "(2,4,6)");
  EXPECT_EQ(vv1 * vv2, 140.0);
  EXPECT_EQ(vv2 * vv1, 140.0);
  EXPECT_EQ(cross_prod(vv1, vv2).to_string(), "(0,0,0)");
  x = Vector3d(2, -1, 3);
  EXPECT_EQ(cross_prod(x, vv1).to_string(), "(-9,-3,5)");
  EXPECT_EQ(cross_prod(vv1, x).to_string(), "(9,3,-5)");
  EXPECT_EQ(vv1.norm_sq(), 14.0);
  EXPECT_EQ(Vector3d(3, 4, 0).norm(), 5.0);
}

TEST(vector, compare)
{
  Vector3d vv1(1, 2, 3), vv2(10, 20, 30);
  EXPECT_EQ(vv1 == vv1, true);
  EXPECT_EQ(vv1 == vv2, false);
  EXPECT_EQ(vv1 != vv2, true);
  EXPECT_EQ(vv1 != vv1, false);
  EXPECT_EQ(vv1 < vv1, false);
  EXPECT_EQ(vv1 < vv2, true);
  EXPECT_EQ(vv2 < vv1, false);
}

}
