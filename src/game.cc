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

Cell Cell::Rotate(const Cell& pivot, int t) {
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
    for (auto& member : members) {
      auto cell = ReadCell(member.get<object>());
      unit.cells.push_back(cell);
    }
    unit.pivot = ReadCell(o_unit["pivot"].get<object>());
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
  visited.clear();
}
