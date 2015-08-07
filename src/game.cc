#include "game.h"
#include "picojson/picojson.h"

using namespace std;
using namespace picojson;

namespace {

Cell ReadCell(object& o) {
  return Cell((int)o["x"].get<double>(),
              (int)o["y"].get<double>());
}

}  // namespace

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
