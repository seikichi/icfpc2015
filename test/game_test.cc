#include <fstream>
#include <string>
#include <iostream>

#include "gtest/gtest.h"
#include "game.h"

using namespace std;

TEST(GameTest, Init) {
  ifstream ifs("problems/problem_1.json");
  string json;
  getline(ifs, json);

  Game g;
  bool init_result = g.Init(json, 0);
  EXPECT_TRUE(init_result);

  // Check member values of g
  EXPECT_EQ(15, g.h);
  EXPECT_EQ(15, g.w);

  EXPECT_EQ(1, g.initial[4 * 15 + 2]);
  EXPECT_EQ(0, g.initial[5 * 15 + 13]);
  EXPECT_EQ(15 * 15, (int)g.initial.size());

  EXPECT_EQ(1, (int)g.units.size());
  EXPECT_EQ(0, g.units[0].cells[0].x);
  EXPECT_EQ(0, g.units[0].cells[0].y);
  EXPECT_EQ(0, g.units[0].pivot.x);
  EXPECT_EQ(0, g.units[0].pivot.y);

  EXPECT_EQ(100, (int)g.source_seq.size());
}

TEST(GameTest, GenerateSourceSequense) {
  Game g;
  g.GenerateSourceSequense(17, 10, 100000);
  EXPECT_EQ(10, (int)g.source_seq.size());
  vector<int> ans{0, 24107, 16552, 12125, 9427, 13152, 21440, 3383, 6873, 16117};
  for (int i = 0; i < 10; ++i) {
    EXPECT_EQ(ans[i], g.source_seq[i]);
  }
}
