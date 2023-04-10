
#include <gtest/gtest.h>

#include "heap.h"

namespace {

TEST(heap, basic)
{
  Heap heap;

  unsigned int *ui = heap.alloc<unsigned int>(3);

  EXPECT_EQ(heap.memory(AMSC), sizeof(unsigned int) * 3);
  EXPECT_EQ(heap.memory(AQ2M), size_t(0));

  //  memory is zeroed
  EXPECT_EQ(ui[0], 0);
  EXPECT_EQ(ui[1], 0);
  EXPECT_EQ(ui[2], 0);
}

TEST(heap, strdup)
{
  Heap heap;

  char buf[] = "SOMESTRING";
  char *copy = heap.strdup(buf, AQ2M);

  EXPECT_EQ(heap.memory(AMSC), size_t(0));
  EXPECT_EQ(heap.memory(AQ2M), strlen(buf) + 1);

  //  copy is a copy
  buf[0] = 'A';
  EXPECT_EQ(std::string(copy), "SOMESTRING");
}

TEST(heap, mat)
{
  Heap heap;

  double **mat = heap.mat(3, 3);

  EXPECT_EQ(heap.memory(AMSC), sizeof(double) * 9 + sizeof(double *) * 3);

  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      EXPECT_EQ(mat[i][j], 0.0);
    }
  }
}

}

