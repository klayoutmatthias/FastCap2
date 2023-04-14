
#include <gtest/gtest.h>

#include "matrix.h"

namespace {

TEST(matrix, basic)
{
  matrix<3, 3> m;
  EXPECT_EQ(m.is_null(), true);
  EXPECT_EQ(m.to_string(), "(0,0,0)\n(0,0,0)\n(0,0,0)");
  Matrix3d mm = unity<3>();
  EXPECT_EQ(mm.is_null(), false);
  EXPECT_EQ(mm.to_string(), "(1,0,0)\n(0,1,0)\n(0,0,1)");
  EXPECT_EQ(Matrix3d(mm).to_string(), "(1,0,0)\n(0,1,0)\n(0,0,1)");
  mm.set(1, 2, 2.0);
  EXPECT_EQ(mm.to_string(), "(1,0,0)\n(0,1,2)\n(0,0,1)");
  Matrix3d mm2(mm);
  EXPECT_EQ(mm2.to_string(), "(1,0,0)\n(0,1,2)\n(0,0,1)");
}

TEST(matrix, arith)
{
  Matrix3d mm1, mm2;
  mm2 = mm1 = unity<3>();
  mm1.set(1, 2, 2.0);
  mm2.set(1, 2, -1.0);
  Matrix3d x;
  EXPECT_EQ((mm1 + mm2).to_string(), "(2,0,0)\n(0,2,1)\n(0,0,2)");
  x = mm1;
  x += mm2;
  EXPECT_EQ(x.to_string(), "(2,0,0)\n(0,2,1)\n(0,0,2)");
  EXPECT_EQ((mm1 - mm2).to_string(), "(0,0,0)\n(0,0,3)\n(0,0,0)");
  x = mm1;
  x -= mm2;
  EXPECT_EQ(x.to_string(), "(0,0,0)\n(0,0,3)\n(0,0,0)");
  EXPECT_EQ((-mm1).to_string(), "(-1,0,0)\n(0,-1,-2)\n(0,0,-1)");
  EXPECT_EQ((mm1 * 2.0).to_string(), "(2,0,0)\n(0,2,4)\n(0,0,2)");
  EXPECT_EQ((2.0 * mm1).to_string(), "(2,0,0)\n(0,2,4)\n(0,0,2)");
  x = mm1;
  x *= 2.0;
  EXPECT_EQ(x.to_string(), "(2,0,0)\n(0,2,4)\n(0,0,2)");

  //  1 0 0     1 0 0
  //  0 1 2  x  0 1 -1
  //  0 0 1     0 0 1
  EXPECT_EQ((mm1 * mm2).to_string(), "(1,0,0)\n(0,1,1)\n(0,0,1)");

  //  1 0 0     1 0 0
  //  0 1 -1 x  0 1 2
  //  0 0 1     0 0 1
  EXPECT_EQ((mm2 * mm1).to_string(), "(1,0,0)\n(0,1,1)\n(0,0,1)");
}

TEST(matrix, compare)
{
  Matrix3d mm1, mm2;
  mm2 = mm1 = unity<3>();
  mm1.set(1, 2, 2.0);
  mm2.set(1, 2, -1.0);

  EXPECT_EQ(mm1 == mm1, true);
  EXPECT_EQ(mm1 == mm2, false);
  EXPECT_EQ(mm1 != mm2, true);
  EXPECT_EQ(mm1 != mm1, false);
  EXPECT_EQ(mm1 < mm1, false);
  EXPECT_EQ(mm1 < mm2, false);
  EXPECT_EQ(mm2 < mm1, true);
}

TEST(matrix, linalg)
{
  Matrix3d mm1;
  mm1 = unity<3>();
  mm1.set(1, 2, 2.0);

  EXPECT_EQ((mm1 * Vector3d(1, 2, 3)).to_string(), "(1,8,3)");

  matrix<2,3> ma1;
  ma1.set(0, 0, 1.0);
  ma1.set(0, 1, 2.0);
  ma1.set(1, 2, 3.0);
  EXPECT_EQ((ma1 * Vector3d(1, 2, 3)).to_string(), "(5,9)");

  mm1.set(1, 1, -1.0);
  mm1.set(2, 1, -2.0);

  Matrix3d mm2 = inverse(mm1);
  EXPECT_EQ((mm2 * mm1).to_string(), "(1,0,0)\n(0,1,0)\n(0,0,1)");

  Matrix4d mx1;
  mx1.set(0, 0, 2.0);
  mx1.set(0, 1, -1.0);
  mx1.set(0, 2, 12.0);
  mx1.set(0, 3, 10.0);
  mx1.set(1, 0, -2.0);
  mx1.set(1, 1, 0.5);
  mx1.set(1, 2, 7.0);
  mx1.set(1, 3, -3.0);
  mx1.set(2, 0, 1.0);
  mx1.set(2, 1, 2.0);
  mx1.set(2, 2, 5.0);
  mx1.set(2, 3, -4.0);
  mx1.set(3, 0, -3.0);
  mx1.set(3, 1, 10.0);
  mx1.set(3, 2, 8.0);
  mx1.set(3, 3, 2.0);

  Matrix4d mx2 = inverse(mx1);
  EXPECT_EQ((mx2 * mx1).to_string(), "(1,0,0,0)\n(0,1,0,0)\n(0,0,1,0)\n(0,0,0,1)");
}

}
