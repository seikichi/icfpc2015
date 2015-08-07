#pragma once

#include <string>
#include <vector>

typedef std::vector<bool> Board;

struct Cell {
  int x, y;
  Cell() : x(0), y(0) {};
  Cell(int x, int y) : x(x), y(y) {}
  int Lin(int w) { return y * w + x; }
};

struct Unit {
  std::vector<Cell> cells;
  Cell pivot;
};

struct Game {
  int problem_id;
  int h, w;
  Board initial;
  std::vector<Unit> units;
  std::vector<int> source_seq;
  // return true if source_seed_idx < num of source seeds.
  bool Init(std::string json, int source_seed_idx);

  void GenerateSourceSequense(int seed, int length, int mod);
};

struct State {
  // return true if the given command is valid
  void Init(const Game& g);
  bool Command(const Game& g, char c);

  Board board;
  std::vector<bool> visited;
  Cell pivot;
  int rot;
  int source_idx;
  int score;
};
