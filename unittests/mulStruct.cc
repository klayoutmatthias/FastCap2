
#include <gtest/gtest.h>

#include "mulStruct.h"
#include "mulGlobal.h"

#include <sstream>

namespace {

TEST(name, basic)
{
  ssystem sys;

  Name name;
  name.name = sys.heap.strdup("X");

  EXPECT_EQ(name.match("A"), false);
  EXPECT_EQ(name.match("B"), false);
  EXPECT_EQ(name.match("X"), true);
  EXPECT_EQ(std::string(name.last_alias()), "X");

  name.add_alias(&sys, "A");
  EXPECT_EQ(name.match("A"), true);
  EXPECT_EQ(name.match("B"), false);
  EXPECT_EQ(name.match("X"), true);
  EXPECT_EQ(std::string(name.last_alias()), "A");

  name.add_alias(&sys, "B");
  EXPECT_EQ(name.match("A"), true);
  EXPECT_EQ(name.match("B"), true);
  EXPECT_EQ(name.match("X"), true);
  EXPECT_EQ(std::string(name.last_alias()), "B");
}

TEST(ssystem, error)
{
  ssystem sys;

  try {
    sys.error("X");
    EXPECT_EQ(true, false);
  } catch (std::runtime_error &ex) {
    EXPECT_EQ(std::string(ex.what()), "X");
  }
}

TEST(ssystem, conductor_names)
{
  ssystem sys;

  EXPECT_EQ(sys.get_conductor_number("A"), 1);
  EXPECT_EQ(sys.get_conductor_number("B"), 2);
  EXPECT_EQ(sys.get_conductor_number("A"), 1);
  EXPECT_EQ(sys.get_conductor_number("C"), 3);

  const Name *nb_const = ((const ssystem &)sys).conductor_name(2);
  EXPECT_EQ(std::string(nb_const->name), "B");
  EXPECT_EQ(sys.number_of(nb_const), 2);

  Name *nb = sys.conductor_name(2);
  EXPECT_EQ(std::string(nb->name), "B");
  EXPECT_EQ(sys.number_of(nb), 2);

  Name *nx = sys.conductor_name(17);
  EXPECT_EQ(nx == 0, true);

  //  conductor renaming 1: by alias
  nb->add_alias(&sys, "X");
  EXPECT_EQ(sys.get_conductor_number("X"), 2);

  //  conductor renaming 2: by sys.rename_conductor
  sys.rename_conductor("X", "Y");
  EXPECT_EQ(sys.get_conductor_number("X"), 2);
  EXPECT_EQ(sys.get_conductor_number("Y"), 2);

  //  0 has no conductor number
  EXPECT_EQ(sys.number_of((Name *)0), NOTFND);
}

static std::string set2s(const std::set<int> &s)
{
  std::ostringstream os;
  for (auto i = s.begin(); i != s.end(); ++i) {
    if (i != s.begin()) {
      os << ",";
    }
    os << *i;
  }
  return os.str();
}

TEST(ssystem, conductor_name_lists)
{
  ssystem sys;

  EXPECT_EQ(sys.get_conductor_number("A"), 1);
  EXPECT_EQ(sys.get_conductor_number("B1"), 2);
  EXPECT_EQ(sys.get_conductor_number("A"), 1);
  EXPECT_EQ(sys.get_conductor_number("C"), 3);

  Name *nc = sys.conductor_name(3);
  nc->add_alias(&sys, "B2");

  std::set<int> nset;
  nset = sys.get_conductor_number_set("A");
  EXPECT_EQ(set2s(nset), "1");
  nset = sys.get_conductor_number_set("B1");
  EXPECT_EQ(set2s(nset), "2");
  nset = sys.get_conductor_number_set("B2");
  EXPECT_EQ(set2s(nset), "3");
  nset = sys.get_conductor_number_set("B2,B1");
  EXPECT_EQ(set2s(nset), "2,3");

  //  "C" is aliased, so it will no longer match
  try {
    nset = sys.get_conductor_number_set("C");
    EXPECT_EQ(true, false);
  } catch (std::runtime_error &ex) {
    EXPECT_EQ(std::string(ex.what()), "Cannot find conductor name starting with 'C'");
  }

  //  "X" was never there
  try {
    nset = sys.get_conductor_number_set("X");
    EXPECT_EQ(true, false);
  } catch (std::runtime_error &ex) {
    EXPECT_EQ(std::string(ex.what()), "Cannot find conductor name starting with 'X'");
  }

  //  "B" alone is not unique
  try {
    nset = sys.get_conductor_number_set("B");
    EXPECT_EQ(true, false);
  } catch (std::runtime_error &ex) {
    EXPECT_EQ(std::string(ex.what()), "Cannot find unique conductor name starting with 'B'");
  }
}

}

