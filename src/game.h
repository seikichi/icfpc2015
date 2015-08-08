#pragma once

#include <string>
#include <utility>
#include <vector>

typedef std::vector<bool> Board;

struct Cell {
  int x, y;
  Cell() : x(0), y(0) {};
  Cell(int x, int y) : x(x), y(y) {}

  int Lin(int w) { return y * w + x; }

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
  Cell TranslateAdd(Cell o) const;
  // Return coordinate after translation s.t. o is moved to (0, 0).
  Cell TranslateSub(Cell o) const;
};

struct Unit {
  std::vector<Cell> cells;
  // NOTE: This variable is set to (0, 0). Do not use this variable!!
  Cell pivot;

  struct Boundary {
    int xmin, xmax, ymin;
  };
  Boundary GetBoundary() const;
};

struct Game {
  int problem_id;
  int source_seed;
  int h, w;
  Board initial;
  std::vector<Unit> units;
  std::vector<int> period;
  std::vector<int> source_seq;
  // return true if source_seed_idx < num of source seeds.
  bool Init(std::string json, int source_seed_idx);

  void GenerateSourceSequense(int seed, int length, int mod);
  void ComputePeriod();

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

struct State {
  void Init(const Game& g);
  // return true if the given command is valid
  // TODO(ir5): Is it better that the return value is enum instead of bool?
  bool Command(const Game& g, char c);

  Board board;
  std::vector<char> visited; // dimension is w * 3
  Cell pivot;  // pivot location
  int rot;
  int source_idx;
  int score;
  int ls_old;  // the number of lines cleared with the previous unit

 // private:
  // return true if the move is valid
  bool UpdateVisitedAndLock(const Game& g, Cell move);
  void Lock(const Game& g);
  void Reset(const Game& g);
  // return the number of lines cleared with the current unit
  int LineDelete(const Game& g);
};
