#include <fstream>
#include <map>
#include <string>
#include <iostream>

#include "gtest/gtest.h"
#include "game.h"

using namespace std;

namespace {

string ReadAll(const char* file) {
  ifstream ifs(file);
  string all, line;
  while (getline(ifs, line)) all += line;
  return all;
}


std::map<string, char> mp{
  {"W", 'p'},
  {"E", 'b'},
  {"SW", 'a'},
  {"SE", 'l'},
  {"CW", 'd'},
  {"CCW", 'k'},
};

}  // namespace

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
  auto json = ReadAll("problems/problem_1.json");

  Game g;
  bool init_result = g.Init(json, 0, {});
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

TEST(UnitTest, GetSpawnPos) {
  auto json = ReadAll("test/game_test/get_spawn_pos.json");

  Game g;
  g.Init(json, 0, {});
  Unit unit = g.units[0];
  int w = 10;
  auto spawn_pos = unit.GetSpawnPos(w);

  EXPECT_EQ(5, spawn_pos.x);
  EXPECT_EQ(1, spawn_pos.y);
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
  auto json = ReadAll("test/game_test/compute_period.json");

  Game g;
  g.Init(json, 0, {});

  EXPECT_EQ(1, g.period[0]);
  EXPECT_EQ(2, g.period[1]);
  EXPECT_EQ(3, g.period[2]);
  EXPECT_EQ(6, g.period[3]);
}

TEST(StateTest, Init) {
  auto json = ReadAll("test/game_test/state_init.json");

  Game g;
  g.Init(json, 0, {});

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

  auto json = ReadAll("test/game_test/state_init.json");

  Game g;
  g.Init(json, 0, {});

  State s;
  s.Init(g);

  for (int x = 0; x < w; ++x)
  for (int y = 0; y < h; ++y) {
    s.SetCell(g, Cell(x, y), (f[y][x] == '1') ? 1 : 0);
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

TEST(StateTest, Command) {
  auto json = ReadAll("test/game_test/state_command.json");

  Game g;
  g.Init(json, 0, {});

  State s;
  s.Init(g);

  vector<string> commands{
    "SE",
    "SW",
    "W",
    "E",
    "SE",
    "SW",
    "W"
  };

  for (int i = 0; i < 3; ++i) {
    EXPECT_EQ(0, s.source_idx);
    EXPECT_EQ(0, s.score);

    char c = mp[commands[i]];
    auto res = s.Command(g, c);
    EXPECT_NE(ERROR, res);
  }
  EXPECT_EQ(1, s.source_idx);
  EXPECT_EQ(1, s.score);

  for (int i = 3; i < 7; ++i) {
    EXPECT_EQ(1, s.source_idx);
    EXPECT_EQ(1, s.score);

    char c = mp[commands[i]];
    auto res = s.Command(g, c);
    EXPECT_NE(ERROR, res);
  }
  EXPECT_EQ(2, s.source_idx);
  EXPECT_EQ(1 + 1 + 100, s.score);
}

TEST(StateTest, CommandRotation) {
  auto json = ReadAll("test/game_test/state_rotation.json");

  Game g;
  g.Init(json, 0, {});

  State s;
  s.Init(g);

  vector<string> commands{
    "E",  // 0
    "CW",
    "CW",
    "CW",
    "SE",
    "SW",
    "CCW", // 6

    "E",  // 7
    "CW",
    "CW",
    "CW",
    "E",
    "CCW",
    "CCW",
    "SW",
    "CW", // 15
  };

  for (int i = 0; i < 7; ++i) {
    EXPECT_EQ(0, s.source_idx);
    EXPECT_EQ(0, s.score);

    char c = mp[commands[i]];
    auto res = s.Command(g, c);
    EXPECT_NE(ERROR, res);
  }
  EXPECT_EQ(1, s.source_idx);
  EXPECT_EQ(2, s.score);

  for (int i = 7; i < 16; ++i) {
    EXPECT_EQ(1, s.source_idx);
    EXPECT_EQ(2, s.score);

    char c = mp[commands[i]];
    auto res = s.Command(g, c);
    EXPECT_NE(ERROR, res);
  }
  EXPECT_EQ(2, s.source_idx);
  EXPECT_EQ(2 + 2 + 100, s.score);
}

TEST(StateTest, GameOver) {
  auto json = ReadAll("test/game_test/state_command.json");

  Game g;
  g.Init(json, 0, {});

  State s;
  s.Init(g);

  auto res1 = s.Command(g, mp["W"]);
  EXPECT_EQ(LOCK, res1);

  auto res2 = s.Command(g, mp["SW"]);
  EXPECT_EQ(GAMEOVER, res2);
}

TEST(StateTest, Clear) {
  auto json = ReadAll("test/game_test/state_clear.json");

  Game g;
  g.Init(json, 0, {});

  State s;
  s.Init(g);

  auto res1 = s.Command(g, mp["SW"]);
  EXPECT_EQ(LOCK, res1);

  auto res2 = s.Command(g, mp["SW"]);
  EXPECT_EQ(LOCK, res2);

  auto res3 = s.Command(g, mp["SW"]);
  EXPECT_EQ(CLEAR, res3);
}

TEST(StateTest, Error) {
  auto json = ReadAll("test/game_test/state_clear.json");

  Game g;
  g.Init(json, 0, {});

  State s;
  s.Init(g);

  auto res1 = s.Command(g, mp["CW"]);
  EXPECT_EQ(ERROR, res1);

  auto res2 = s.Command(g, mp["W"]);
  EXPECT_EQ(LOCK, res2);
}

TEST(StateTest, PowerEi) {
  auto json = ReadAll("test/game_test/state_power.json");

  Game g;
  g.Init(json, 0, {});

  State s;
  s.Init(g);

  {
    string commands("ei! ei!");

    for (int i = 0; i < 3; ++i) {
      EXPECT_EQ(0, s.source_idx);
      EXPECT_EQ(0, s.score);

      auto res = s.Command(g, commands[i]);
      EXPECT_EQ(MOVE, res);
    }
    EXPECT_EQ(300 + 6, s.score);

    for (int i = 3; i < 7; ++i) {
      EXPECT_EQ(0, s.source_idx);
      EXPECT_EQ(300 + 6, s.score);

      auto res = s.Command(g, commands[i]);
      EXPECT_EQ(MOVE, res);
    }
    EXPECT_EQ(300 + 6 + 6, s.score);
  }
}
