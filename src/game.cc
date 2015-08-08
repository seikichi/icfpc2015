#include <algorithm>
#include <cassert>
#include "picojson/picojson.h"

#include "game.h"

using namespace std;
using namespace picojson;


namespace {

Cell ReadCell(object& o) {
  return Cell((int)o["x"].get<double>(),
              (int)o["y"].get<double>());
}

// return floor(a / b) (assuming that b > 0)
int DivFloor(int a, int b) {
  return (a >= 0) ? (a / b) : -((-a + b - 1) / b);
}

}  // namespace

Cell Cell::Rotate(const Cell& pivot, int t) const {
  // Normalize pivot to (0, 0)
  Cell res(x - pivot.x, y - pivot.y);

  // Transform res to cartesian coordinate
  res.x += -DivFloor(res.y, 2);

  // Calculate distance from (0, 0) to res
  int r = ((res.x >= 0) ^ (res.y >= 0)) ? max(abs(res.x), abs(res.y))
                                        : (abs(res.x) + abs(res.y));

  // Calculate offset and position
  int of = -1, pos = -1;
  if (res.y == -r)              { pos = 0; of = r - res.x; }
  if (res.y <= 0 && res.x <= 0) { pos = 1; of = -res.x; }
  if (res.x == -r)              { pos = 2; of = res.y; }
  if (res.y == r)               { pos = 3; of = r + res.x; }
  if (res.y >= 0 && res.x >= 0) { pos = 4; of = res.x; }
  if (res.x == r)               { pos = 5; of = -res.y; }
  assert(pos >= 0);

  // Perform rotation
  pos = (pos + t) % 6;
  if (pos == 0) { res.x = r - of;  res.y = -r; }
  if (pos == 1) { res.x = -of;     res.y = -r + of; }
  if (pos == 2) { res.x = -r;      res.y = of; }
  if (pos == 3) { res.x = -r + of; res.y = r; }
  if (pos == 4) { res.x = of;      res.y = r - of; }
  if (pos == 5) { res.x = r;       res.y = -of; }

  // Transform res to hex coordinate
  res.x += DivFloor(res.y, 2);

  // Finally denormalize the pivot
  res.x += pivot.x;
  res.y += pivot.y;

  return res;
}

Cell Cell::TranslateAdd(Cell o) const {
  Cell res(x - DivFloor(y, 2), y);
  o.x += -DivFloor(o.y, 2);

  res.x += o.x;
  res.y += o.y;

  res.x += DivFloor(res.y, 2);
  return res;
}

Cell Cell::TranslateSub(Cell o) const {
  Cell res(x - DivFloor(y, 2), y);
  o.x += -DivFloor(o.y, 2);

  res.x -= o.x;
  res.y -= o.y;

  res.x += DivFloor(res.y, 2);
  return res;
}

Unit::Boundary Unit::GetBoundary() const {
  Unit::Boundary ret{9999, -9999, 9999};
  for (const auto& cell : cells) {
    ret.xmin = min(ret.xmin, cell.x);
    ret.xmax = max(ret.xmax, cell.x);
    ret.ymin = min(ret.ymin, cell.y);
  }
  return ret;
}

bool Game::Init(std::string json, int source_seed_idx) {
  value v;
  string err = parse(v, json);
  if (!err.empty()) {
    cerr << err << endl;
    return false;
  }

  auto o = v.get<object>();
  problem_id = (int)o["id"].get<double>();
  w = (int)o["width"].get<double>();
  h = (int)o["height"].get<double>();

  const auto& a_units = o["units"].get<picojson::array>();
  for (auto& v_unit : a_units) {
    Unit unit;
    auto o_unit = v_unit.get<object>();
    auto members = o_unit["members"].get<picojson::array>();
    auto pivot = ReadCell(o_unit["pivot"].get<object>());
    for (auto& member : members) {
      auto cell = ReadCell(member.get<object>());
      auto normalized_cell = cell.TranslateSub(pivot);
      unit.cells.push_back(normalized_cell);
    }
    unit.pivot = Cell(0, 0);  // NOTE: Do not use this
    units.push_back(unit);
  }

  initial.resize(h * w);
  auto a_filled = o["filled"].get<picojson::array>();
  for (auto& v_f : a_filled) {
    Cell pos = ReadCell(v_f.get<object>());
    initial[pos.Lin(w)] = 1;
  }

  std::vector<int> source_seeds;
  auto a_source_seeds = o["sourceSeeds"].get<picojson::array>();
  for (auto& v_source_seed : a_source_seeds) {
    source_seeds.push_back((int)v_source_seed.get<double>());
  }

  // Generate source_seq
  if (source_seed_idx >= (int)source_seeds.size()) {
    // invalid
    return false;
  }
  source_seed = source_seeds[source_seed_idx];
  int source_length = (int)o["sourceLength"].get<double>();
  GenerateSourceSequense(source_seeds[source_seed_idx],
                         source_length,
                         units.size());

  // Compute period of each unit
  ComputePeriod();

  return true;
}

void Game::GenerateSourceSequense(int seed, int length, int mod) {
  const uint32_t mul = 1103515245;
  const uint32_t add = 12345;
  uint32_t s = seed;
  source_seq.resize(length);
  for (int i = 0; i < length; ++i) {
    uint32_t a = (s >> 16) & ((1 << 15) - 1);
    source_seq[i] = (int)(a) % mod;
    s = s * mul + add;
  }
}

void Game::ComputePeriod() {
  period.clear();

  for (const auto& unit : units) {
    std::vector<Cell> s1 = unit.cells, s2;
    sort(s1.begin(), s1.end());
    s2 = s1;

    for (int p = 1; ; ++p) {
      for (auto& c : s2) c = c.Rotate(unit.pivot, 1);
      sort(s2.begin(), s2.end());

      if (s1 == s2) {
        period.push_back(p);
        break;
      }
    }
  }
}

void State::Init(const Game& g) {
  board = g.initial;
  visited.assign(g.w * 3, 0);
  rot = 0;
  source_idx = 0;
  score = 0;
  ls_old = 0;

  Reset(g);
}

bool State::Command(const Game& g, char c) {
  const string command_chars[] = {
    "p'!.03",
    "bcefy2",
    "aghij4",
    "lmno 5",
    "dqrvz1",
    "kstuwx",
    "\t\n\r",
  };
  const string command_list[] = {
    "W", "E", "SW", "SE", "RCW", "RCCW", "I",
  };

  string command;
  for (int i = 0; i < 7; ++i) {
    if (command_chars[i].find(c) != string::npos) {
      command = command_list[i];
    }
  }

  if (command.empty()) {
    cerr << "AHOKA:Invalid command c=" << c << endl;
    exit(1);
  }

  if (command == "W") {
    return UpdateVisitedAndLock(g, Cell(-1, 0));
  }

  if (command == "E") {
    return UpdateVisitedAndLock(g, Cell(+1, 0));
  }

  if (command == "SW") {
    return UpdateVisitedAndLock(g, Cell(-1, 1));
  }

  if (command == "SE") {
    return UpdateVisitedAndLock(g, Cell(0, 1));
  }

  if (command == "RCW") {
    return UpdateRotAndLock(g, -1);
  }

  if (command == "RCCW") {
    return UpdateRotAndLock(g, +1);
  }

  if (command == "I") {
    // just ignore
    return true;
  }

  cerr << "?????" << endl;
  exit(1);
  return false;
}

bool State::UpdateVisitedAndLock(const Game& g, Cell move) {
  assert(move.y >= 0);
  // Check visited
  pivot = pivot.TranslateAdd(move);

  if (move.y) {
    visited.assign(3 * g.w, 0);
  }

  if (visited[pivot.x + g.w] & (1 << rot)) {
    // Invalid operation
    pivot = pivot.TranslateSub(move);
    return false;
  }
  visited[pivot.x + g.w] |= 1 << rot;

  // Check Lock
  const auto& unit = g.CurrentUnit(source_idx);
  for (const auto& cell : unit.cells) {
    Cell c = cell.Rotate(Cell(0, 0), rot).TranslateAdd(pivot);
    if (c.x < 0 || c.y < 0 || c.x >= g.w || c.y >= g.h || board[c.Lin(g.w)]) {
      // The unit must be locked, revert the pivot and terminate
      pivot = pivot.TranslateSub(move);
      Lock(g);
      return true;
    }
  }

  return true;
}

bool State::UpdateRotAndLock(const Game& g, int dir) {
  int p = g.CurrentPeriod(source_idx);
  rot = (rot + dir + p) % p;

  if (visited[pivot.x + g.w] & (1 << rot)) {
    // Invalid operation
    rot = (rot - dir + p) % p;
    return false;
  }
  visited[pivot.x + g.w] |= 1 << rot;

  // NOTE: Copy & Paste is good
  const auto& unit = g.CurrentUnit(source_idx);
  for (const auto& cell : unit.cells) {
    Cell c = cell.Rotate(Cell(0, 0), rot).TranslateAdd(pivot);
    if (c.x < 0 || c.y < 0 || c.x >= g.w || c.y >= g.h || board[c.Lin(g.w)]) {
      // Revert and lock
      rot = (rot - dir + p) % p;
      Lock(g);
      return true;
    }
  }

  return true;
}

void State::Lock(const Game& g) {
  const auto& unit = g.CurrentUnit(source_idx);
  for (const auto& cell : unit.cells) {
    Cell c = cell.Rotate(Cell(0, 0), rot).TranslateAdd(pivot);
    assert(board[c.Lin(g.w)] == 0);
    board[c.Lin(g.w)] = 1;
  }

  // Line deletion
  int ls = LineDelete(g);

  source_idx++;
  rot = 0;
  // Compute and update score
  int size = (int)unit.cells.size();
  int points = size + 100 * (1 + ls) * ls / 2;
  int line_bonus = (ls_old > 1) ? (ls_old - 1) * points / 10 : 0;
  score += points + line_bonus;

  ls_old = ls;
  visited.assign(3 * g.w, 0);
  Reset(g);
}

void State::Reset(const Game& g) {
  if (source_idx >= (int)g.source_seq.size()) {
    return;
  }
  auto& unit = g.CurrentUnit(source_idx);
  auto b = unit.GetBoundary();
  int bw = b.xmax - b.xmin + 1;
  Cell top_left_u(b.xmin, b.ymin);
  Cell top_left_o((g.w - bw) / 2, 0);

  pivot = top_left_o.TranslateSub(top_left_u);
}

int State::LineDelete(const Game& g) {
  int ls = 0;
  int ty = g.h - 1;
  for (int y = g.h - 1; y >= 0; --y) {
    bool del = 1;
    for (int x = 0; x < g.w; ++x) del &= board[Cell(x, y).Lin(g.w)];
    if (!del) {
      if (ty != y) {
        for (int x = 0; x < g.w; ++x) {
          board[Cell(x, ty).Lin(g.w)] = board[Cell(x, y).Lin(g.w)];
        }
      }
      ty--;
    } else {
      ls++;
    }
  }

  // zero-fill
  for (int y = ty; y >= 0; --y) {
    for (int x = 0; x < g.w; ++x) board[Cell(x, y).Lin(g.w)] = 0;
  }
  return ls;
}
