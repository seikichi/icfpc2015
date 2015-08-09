#include <algorithm>
#include <cassert>
#include <fstream>
#include "picojson/picojson.h"

#include "game.h"

using namespace std;
using namespace picojson;


namespace {

Cell ReadCell(object& o) {
  return Cell((int)o["x"].get<double>(),
              (int)o["y"].get<double>());
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

Cell Unit::GetSpawnPos(int w) const {
  int ymin = 9999999;
  for (const auto& cell : cells) ymin = min(ymin, cell.y);

  // TODO: Maybe a little slow
  for (int px = -w; px <= w; ++px) {
    int l_len = 99999, r_len = 99999;
    Cell mov(px, -ymin);
    for (const auto& cell : cells) {
      l_len = min(l_len, cell.TranslateAdd(mov).x);
      r_len = min(r_len, w - 1 - cell.TranslateAdd(mov).x);
    }

    if (l_len == r_len || (l_len + 1 == r_len)) {
      return Cell(px, -ymin);
    }
  }

  // Error
  cerr << "Unit::GetSpawnPos failed... ('_`)" << endl;
  exit(1);
  return Cell(-66666666, -6666666);
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
    unit.spawn_pos[w] = unit.GetSpawnPos(w); // memoize spown pos
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
  num_source_seeds = (int)source_seeds.size();

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

  // Init power PMA and power length
  SetPowerInfo();

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

void Game::SetPowerInfo() {
  ifstream ifs("ini/power.txt");
  string line;

  vector<string> powers;
  while (getline(ifs, line)) {
    if (line == "") continue;
    // NOTE: Convert the given characters to lowercase
    transform(line.begin(), line.end(), line.begin(), ::tolower);
    powers.push_back(line);
    power_len.push_back((int)line.size());
  }

  power_pma.Build(powers);
}

void State::Init(const Game& g) {
  board = g.initial;
  rot = 0;
  source_idx = 0;
  score = 0;
  ls_old = 0;
  gameover = 0;

  pma_node = g.power_pma.GetInitialNode();
  used_power = util::AcceptIndex();
  fill_count.assign(g.h, 0);
  for (int y = 0; y < g.h; ++y) {
    for (int x = 0; x < g.w; ++x) {
      fill_count[y] += g.initial[Cell(x, y).Lin(g.w)] ? 1 :0;
    }
  }

  // Spawn the first unit
  Reset(g);
}

bool State::IsGameOver(const Game&) const {
  return gameover;
}

bool State::IsClear(const Game& g) const {
  return source_idx >= (int)g.source_seq.size();
}

CommandResult State::Command(const Game& g, char c) {
  if (IsClear(g)) {
    return CLEAR;
  }
  // NOTE: Return LOCK instead of GAMEOVER if a command leads to game over result.
  if (IsGameOver(g)) {
    return GAMEOVER;
  }

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
    return UpdateVisitedAndLock(g, Cell(-1, 0), c);
  }

  if (command == "E") {
    return UpdateVisitedAndLock(g, Cell(+1, 0), c);
  }

  if (command == "SW") {
    return UpdateVisitedAndLock(g, Cell(-1, 1), c);
  }

  if (command == "SE") {
    return UpdateVisitedAndLock(g, Cell(0, 1), c);
  }

  if (command == "RCW") {
    return UpdateRotAndLock(g, -1, c);
  }

  if (command == "RCCW") {
    return UpdateRotAndLock(g, +1, c);
  }

  if (command == "I") {
    // just ignore
    return MOVE;
  }

  cerr << "?????" << endl;
  exit(1);
  return ERROR;
}

CommandResult State::UpdateVisitedAndLock(const Game& g, Cell move, char c) {
  assert(move.y >= 0);
  // Check visited
  pivot = pivot.TranslateAdd(move);

  if (move.y) {
    visited.assign(3 * g.w, 0);
  }

  if (visited[pivot.x + g.w] & (1 << rot)) {
    // Invalid operation
    pivot = pivot.TranslateSub(move);
    return ERROR;
  }
  visited[pivot.x + g.w] |= 1 << rot;
  UpdatePowerPMA(g, c);

  // Check Lock
  const auto& unit = g.CurrentUnit(source_idx);
  for (const auto& cell : unit.cells) {
    Cell c = cell.Rotate(Cell(0, 0), rot).TranslateAdd(pivot);
    if (c.x < 0 || c.y < 0 || c.x >= g.w || c.y >= g.h || board[c.Lin(g.w)]) {
      // The unit must be locked, revert the pivot and terminate
      pivot = pivot.TranslateSub(move);
      Lock(g);
      return LOCK;
    }
  }

  return MOVE;
}

CommandResult State::UpdateRotAndLock(const Game& g, int dir, char c) {
  int p = g.CurrentPeriod(source_idx);
  rot = (rot + dir + p) % p;

  if (visited[pivot.x + g.w] & (1 << rot)) {
    // Invalid operation
    rot = (rot - dir + p) % p;
    return ERROR;
  }
  visited[pivot.x + g.w] |= 1 << rot;
  UpdatePowerPMA(g, c);

  // NOTE: Copy & Paste is good
  const auto& unit = g.CurrentUnit(source_idx);
  for (const auto& cell : unit.cells) {
    Cell c = cell.Rotate(Cell(0, 0), rot).TranslateAdd(pivot);
    if (c.x < 0 || c.y < 0 || c.x >= g.w || c.y >= g.h || board[c.Lin(g.w)]) {
      // Revert and lock
      rot = (rot - dir + p) % p;
      Lock(g);
      return LOCK;
    }
  }

  return MOVE;
}

void State::UpdatePowerPMA(const Game& g, char c) {
  auto accept_index = g.power_pma.UpdateNode(c, pma_node);

  // Update score
  int power_score = 0;
  for (int i = 0; i < (int)accept_index.size(); ++i) {
    if (accept_index[i]) {
      if (!used_power[i]) {
        power_score += 300;  // Bonus
        used_power[i] = 1;
      }
      power_score += 2 * g.power_len[i];
    }
  }
  score += power_score;
}

void State::Lock(const Game& g) {
  const auto& unit = g.CurrentUnit(source_idx);
  for (const auto& cell : unit.cells) {
    Cell c = cell.Rotate(Cell(0, 0), rot).TranslateAdd(pivot);
    const int idx = c.Lin(g.w);
    assert(board[idx] == 0);
    board[idx] = 1;
    fill_count[c.y]++;
  }

  // Line deletion
  int ls = LineDelete(g);

  // Compute and update score
  int size = (int)unit.cells.size();
  int points = size + 100 * (1 + ls) * ls / 2;
  int line_bonus = (ls_old > 1) ? (ls_old - 1) * points / 10 : 0;
  score += points + line_bonus;

  ls_old = ls;

  // Spawn the next unit
  source_idx++;
  rot = 0;
  Reset(g);
}

void State::Reset(const Game& g) {
  if (source_idx >= (int)g.source_seq.size()) {
    return;
  }
  const auto& unit = g.CurrentUnit(source_idx);
  pivot = unit.spawn_pos.find(g.w)->second;

  // Check game over
  for (const auto& cell : unit.cells) {
    Cell c = cell.Rotate(Cell(0, 0), rot).TranslateAdd(pivot);
    assert(!(c.x < 0 || c.y < 0 || c.x >= g.w || c.y >= g.h)); // something is strange!
    if (board[c.Lin(g.w)]) {
      gameover = 1;
      return;
    }
  }

  // Fill visited
  visited.assign(g.w * 3, 0);
  visited[pivot.x + g.w] |= 1 << rot;
}

int State::LineDelete(const Game& g) {
  // pruning
  bool exist = 0;
  for (int y = 0; y < g.h; ++y) {
    if (fill_count[y] == g.w) {
      exist = 1;
      break;
    }
  }
  if (!exist) return 0;

  int ls = 0;
  int ty = g.h - 1;
  for (int y = g.h - 1; y >= 0; --y) {
    bool del = 1;
    for (int x = 0; x < g.w; ++x) del &= board[Cell(x, y).Lin(g.w)];
    if (!del) {
      if (ty != y) {
        // info(ty) <- info(y)
        for (int x = 0; x < g.w; ++x) {
          board[Cell(x, ty).Lin(g.w)] = board[Cell(x, y).Lin(g.w)];
        }
        fill_count[ty] = fill_count[y];
      }
      ty--;
    } else {
      ls++;
    }
  }

  // zero-fill
  for (int y = ty; y >= 0; --y) {
    for (int x = 0; x < g.w; ++x) board[Cell(x, y).Lin(g.w)] = 0;
    fill_count[ty] = 0;
  }
  return ls;
}

vector<Cell> State::GetCurrentUnitCells(const Game& game) const {
  const Unit& unit = game.CurrentUnit(source_idx);
  std::vector<Cell> ret;
  for (const Cell& cell : unit.cells) {
    ret.push_back(cell.GetCurrentPos(rot, pivot));
  }
  return ret;
}
