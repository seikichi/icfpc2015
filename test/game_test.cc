#include <fstream>
#include <string>
#include <iostream>

#include "gtest/gtest.h"
#include "game.h"

using namespace std;

TEST(CellTest, Rotate) {
  // do not rotate
  for (int x = -3; x <= 3; ++x)
  for (int y = -3; y <= 3; ++y) {
    Cell c(x, y);
    Cell after = c.Rotate(Cell(1, -1), 0);
    EXPECT_EQ(x, after.x);
    EXPECT_EQ(y, after.y);
  }

  // rotate
  {
    Cell center(2, 6);
    Cell now(3, 6);
    
    now = now.Rotate(center, 1);
    EXPECT_EQ(2, now.x);
    EXPECT_EQ(5, now.y);

    now = now.Rotate(center, 1);
    EXPECT_EQ(1, now.x);
    EXPECT_EQ(5, now.y);

    now = now.Rotate(center, 1);
    EXPECT_EQ(1, now.x);
    EXPECT_EQ(6, now.y);

    now = now.Rotate(center, 1);
    EXPECT_EQ(1, now.x);
    EXPECT_EQ(7, now.y);

    now = now.Rotate(center, 1);
    EXPECT_EQ(2, now.x);
    EXPECT_EQ(7, now.y);

    now = now.Rotate(center, 1);
    EXPECT_EQ(3, now.x);
    EXPECT_EQ(6, now.y);
  }
}

TEST(CellTest, Translate) {
  Cell c1(123, 456);
  Cell c2(-999, 88);
  EXPECT_EQ(c1.x, c1.TranslateAdd(c2).TranslateSub(c2).x);
  EXPECT_EQ(c1.y, c1.TranslateAdd(c2).TranslateSub(c2).y);
  EXPECT_EQ(c2.x, c2.TranslateAdd(c1).TranslateSub(c1).x);
  EXPECT_EQ(c2.y, c2.TranslateAdd(c1).TranslateSub(c1).y);
}

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

TEST(GameTest, ComputePeriod) {
  ifstream ifs("test/game_test/compute_period.json");
  string json, line;
  while (getline(ifs, line)) json += line;

  Game g;
  g.Init(json, 0);

  EXPECT_EQ(1, g.period[0]);
  EXPECT_EQ(2, g.period[1]);
  EXPECT_EQ(3, g.period[2]);
  EXPECT_EQ(6, g.period[3]);
}

TEST(StateTest, Init) {
  ifstream ifs("test/game_test/state_init.json");
  string json, line;
  while (getline(ifs, line)) json += line;

  Game g;
  g.Init(json, 0);

  State s;
  s.Init(g);
  EXPECT_EQ(2, s.pivot.x);
  EXPECT_EQ(3, s.pivot.y);
}

TEST(StateTest, LineDelete) {
  const int w = 5, h = 10;
  string f[h] = {
    "11...",
    "..1.1",
    ".....",
    "11111",
    ".1...",
    ".....",
    "..1..",
    "11111",
    "11111",
    "1.111",
  };
  string ans[h] = {
    ".....",
    ".....",
    ".....",
    "11...",
    "..1.1",
    ".....",
    ".1...",
    ".....",
    "..1..",
    "1.111",
  };

  ifstream ifs("test/game_test/state_init.json");
  string json, line;
  while (getline(ifs, line)) json += line;

  Game g;
  g.Init(json, 0);

  State s;
  s.Init(g);

  for (int x = 0; x < w; ++x)
  for (int y = 0; y < h; ++y) {
    s.board[Cell(x, y).Lin(w)] = (f[y][x] == '1') ? 1 : 0;
  }
  int ls = s.LineDelete(g);

  EXPECT_EQ(3, ls);
  for (int x = 0; x < w; ++x)
  for (int y = 0; y < h; ++y) {
    int idx = Cell(x, y).Lin(w);
    bool expect = ans[y][x] == '1' ? 1 : 0;
    EXPECT_EQ(expect,
              s.board[idx]);
  }
}
