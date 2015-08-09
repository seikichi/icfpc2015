#pragma once

#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <bitset>

#include "util.h"

typedef std::vector<bool> Board;

namespace {

// (Auxiliary function)
// Return floor(a / b) (assuming that b > 0)
inline int DivFloor(int a, int b) {
  return (a >= 0) ? (a / b) : -((-a + b - 1) / b);
}

}  // namespace

struct Cell {
  int x, y;
  Cell() : x(0), y(0) {};
  Cell(int x, int y) : x(x), y(y) {}

  inline int Lin(int w) { return y * w + x; }

  bool operator<(const Cell& rhs) const {
    return std::make_pair(x, y) < std::make_pair(rhs.x, rhs.y);
  }
  bool operator==(const Cell& rhs) const {
    return std::make_pair(x, y) == std::make_pair(rhs.x, rhs.y);
  }

  // t must be specified in ccw order
  Cell Rotate(const Cell& pivot, int t) const;

  // Return coordinate after translation s.t. (0, 0) is moved to o.
  // (Coordinate must be specified in zigzag-coordinate(?))
  inline Cell TranslateAdd(Cell o) const {
    Cell res(x - DivFloor(y, 2), y);
    o.x += -DivFloor(o.y, 2);

    res.x += o.x;
    res.y += o.y;

    res.x += DivFloor(res.y, 2);
    return res;
  }

  // Return coordinate after translation s.t. o is moved to (0, 0).
  inline Cell TranslateSub(Cell o) const {
    Cell res(x - DivFloor(y, 2), y);
    o.x += -DivFloor(o.y, 2);

    res.x -= o.x;
    res.y -= o.y;

    res.x += DivFloor(res.y, 2);
    return res;
  }

  inline Cell GetCurrentPos(int rot, const Cell &pivot) const {
    return Rotate(Cell(0, 0), rot).TranslateAdd(pivot);
  }
};

struct Unit {
  std::vector<Cell> cells;
  // NOTE: This variable is set to (0, 0). Do not use this variable!!
  Cell pivot;
  std::map<int, Cell> spawn_pos; // [w] => spawn pos

  Cell GetSpawnPos(int w) const;
};

struct Game {
  int problem_id;
  int source_seed;
  int h, w;
  Board initial;
  std::vector<Unit> units;
  std::vector<int> period;
  std::vector<int> source_seq;
  int num_source_seeds;
  util::PMA power_pma;
  std::vector<int> power_len;

  // return true if source_seed_idx < num of source seeds.
  bool Init(std::string json, int source_seed_idx);

  void GenerateSourceSequense(int seed, int length, int mod);
  void ComputePeriod();
  void SetPowerInfo();

  Unit& CurrentUnit(int source_idx) {
    return units[source_seq[source_idx]];
  }
  const Unit& CurrentUnit(int source_idx) const {
    return units[source_seq[source_idx]];
  }
  int CurrentPeriod(int source_idx) const {
    return period[source_seq[source_idx]];
  }

};

enum CommandResult {
  MOVE,
  LOCK,
  ERROR,
  CLEAR,
  GAMEOVER,
};

struct State {
  void Init(const Game& g);
  // return true if the given command is valid
  CommandResult Command(const Game& g, char c);
  bool IsGameOver(const Game& g) const;
  bool IsClear(const Game& g) const;

  Board board;
  std::vector<char> visited; // dimension is w * 3
  Cell pivot;  // pivot location
  int rot;
  int source_idx;
  int score;
  int ls_old;  // the number of lines cleared with the previous unit
  bool gameover;
  util::Node* pma_node;  // for power-of-phrase check
  util::AcceptIndex used_power;  // set of used power

 // private:
  // return true if the move is valid
  // TODO
  CommandResult UpdateVisitedAndLock(const Game& g, Cell move, char c);
  CommandResult UpdateRotAndLock(const Game& g, int dir, char c);
  void UpdatePowerPMA(const Game& g, char c);
  void Lock(const Game& g);
  void Reset(const Game& g);
  // return the number of lines cleared with the current unit
  int LineDelete(const Game& g);
  std::vector<Cell> GetCurrentUnitCells(const Game& gaem) const;

  // For debug use
  void PrintBoard(const Game& g) const {
    for (int y = 0; y < g.h; ++y) {
      if (y % 2) std::cerr << " ";
      for (int x = 0; x < g.w; ++x) {
        char z = board[Cell(x, y).Lin(g.w)] ? '#' : '.';
        std::cerr << z << " ";
      }
      std::cerr << std::endl;
    }
  }
};
