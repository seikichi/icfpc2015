#include "submarineai.h"
#include "evaluator.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <queue>
#include <cassert>
#include <cmath>
#include <sys/time.h>
#include <set>
using namespace std;

namespace {

struct Item {
  SmallState sstate;
  string commands;
  int priority;
  Item(const Game& game, const State& initial_state, const SmallState& sstate, const string& commands)
    : sstate(sstate), commands(commands), priority(0) {
    const Unit& unit = game.CurrentUnit(initial_state.source_idx);
    for (const auto& cell : unit.cells) {
        Cell c = cell.Rotate(Cell(0, 0), sstate.rot).TranslateAdd(sstate.pivot);
        priority = max((int)(fabs(c.x - game.w/2.0) + (double)game.w * c.y / game.h), priority);
    }
  }

  bool operator<(const Item& rhs) const {
    return priority < rhs.priority;
  }
};

struct ItemAnnealing {
  SmallState sstate;
  string commands;
  int priority;
  ItemAnnealing(const Game& , const State& , const SmallState& sstate, const string& commands, int base_score, const Cell& target_cell)
    : sstate(sstate), commands(commands), priority(0) {
      int dist = abs(target_cell.x - sstate.pivot.x) + abs(target_cell.y - sstate.pivot.y);
      priority = (sstate.score - base_score) * 100 + sstate.pma_node->pos * 100 - dist;
  }
  bool operator<(const ItemAnnealing& rhs) const {
    return priority < rhs.priority;
  }
};

// int evaluateScore_0815_1917(const Game&, const State& state, const State&) {
//   return state.pivot.y;
// }

long long getTime() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (long long)tv.tv_sec * 1000000 + (long long)tv.tv_usec;
}

};


TleState SubmarineAI::ShouldExitLoop(long long unit_start_time, long long time_limit_per_unit) const {
  long long now = getTime();
  if (now - unit_start_time >= time_limit_per_unit) {
    return TleState::Yellow;
  }
  if (time_keeper->RemainingTimeForTheSeed() <= 0) {
    cerr << "ShouldExitLoop: Red" << endl;
    return TleState::Red;
  }
  return TleState::Green;
}

pair<int, string> SubmarineAI::Step(const Game& game, const State& initial_state, int loop_count) {
  // 1ユニットごとにループ
  const int visited_w = 3 * game.w;
  const int visited_offset_x = game.w;
  const int visited_h = 3 * game.h;
  const int visited_offset_y = game.h;
  vector<bool> visited(visited_h * visited_w * 6, false);

  if (initial_state.IsClear(game) || initial_state.IsGameOver(game)) {
    return make_pair(-1, "");
  }

  int max_score = -1;
  string best_commands = "";
  priority_queue<Item> Q;
  SmallState initial_sstate;
  initial_sstate.Init(initial_state);
  Q.push(Item(game, initial_state, initial_sstate, ""));

  long long start_time = getTime();
  while (!Q.empty()) {
    TleState tle_state = ShouldExitLoop(start_time, time_limit_per_unit_for_initial_search);
    if (tle_state == TleState::Yellow && max_score != -1)
      break;
    else if (tle_state == TleState::Red)
      return make_pair(-1, "");

    const Item item = Q.top(); Q.pop();

    assert(0 <= item.sstate.pivot.x + visited_offset_x);
    assert(item.sstate.pivot.x + visited_offset_x < visited_w);
    assert(0 <= item.sstate.pivot.y + visited_offset_y);
    assert(item.sstate.pivot.y + visited_offset_y < visited_h);
    const int visited_index =
      visited_w * 6 * (item.sstate.pivot.y + visited_offset_y) +
      6 * (item.sstate.pivot.x + visited_offset_x) + item.sstate.rot;

    if (visited[visited_index]) { continue; }
    visited[visited_index] = true;

    const vector<char> commands = {'!', 'e', 'i', ' ', 'd', 'k'};
    for (auto c : commands) {
      SmallState next_sstate = item.sstate;
      const CommandResult result = next_sstate.Command(game, initial_state, c);
      const string next_commands = item.commands + string(1, c);

      assert(result != GAMEOVER && result != CLEAR);
      if (result == ERROR) {
        continue;
      } else if (result == LOCK) {
        auto e = Evaluator::CreateEvaluator();
        const int score = e->evaluate(
            game, initial_state, item.sstate, next_sstate);
        if (score > max_score) {
          max_score = score;
          best_commands = next_commands;
        }
      } else if (result == MOVE) {
        Q.push(Item(game, initial_state, next_sstate, next_commands));
      } else {
        assert(false);
      }
    }
  }

  cerr << "Loop " << loop_count << ": time=" << getTime() - start_time << " usec, total=" << getTime() - time_keeper->seed_start_time << " usec" << endl;

  return make_pair(max_score, best_commands);
}

string SubmarineAI::Annealing(const Game &game, const State& initial_state, string& initial_answer, int loop_count) const {
  int height = 1;
  for (char c : initial_answer) {
    util::Command one_command = util::GetCommand(c);
    if (one_command == util::Command::SOUTH_WEST || one_command == util::Command::SOUTH_EAST) {
      height++;
    }
  }
  if (height < 3) { return initial_answer; }

  string commands = initial_answer;
  SmallState initial_sstate;
  initial_sstate.Init(initial_state);
  SmallState initial_last_sstate = CalcLastState(game, initial_state, initial_sstate, initial_answer);
  double energy = CalcEnergy(game, initial_last_sstate);
  pair<double, string> best_answer = make_pair(energy, initial_answer);
  int counter = 0;
  long long start_time = getTime();
  long long end_time = time_limit_per_unit_for_annealing + start_time;
  while (true) {
    TleState tle_state = ShouldExitLoop(start_time, time_limit_per_unit_for_annealing);
    if (tle_state == TleState::Yellow || tle_state == TleState::Red) { break; }
    long long current_time = getTime();
    double temperature = (double)(end_time - current_time) / (double)(end_time - start_time) + 1e-8;
    string next_commands = ChangePath(game, initial_state, commands, loop_count, temperature, start_time);
    SmallState next_last_sstate = CalcLastState(game, initial_state, initial_sstate, next_commands);
    assert(next_last_sstate.pivot == initial_last_sstate.pivot);
    assert(next_last_sstate.rot == initial_last_sstate.rot);
    double next_energy = CalcEnergy(game, next_last_sstate);
    double probability = CalcProbability(energy, next_energy,  temperature + 1e-8);
    double r = random.next(0.0, 1.0);
    if (r < probability) {
      if (next_energy < energy) {
        best_answer.first = next_energy;
        best_answer.second = next_commands;
      }
      energy = next_energy;
      commands = next_commands;
    }
    counter++;
  }
  cerr << "Anneaing Loop " << loop_count << ": time=" << getTime() - start_time << " count: " << counter << endl;
  return best_answer.second;
}

string SubmarineAI::ChangePath(const Game &game, const State& initial_state, const string& commands, int loop_count, double temperature, long long start_time) const {
  vector<int> down_poss = { 0 };
  for (int i = 0; i < (int)commands.size(); i++) {
    util::Command c = util::GetCommand(commands[i]);
    if (c == util::Command::SOUTH_WEST || c == util::Command::SOUTH_EAST || i == (int)commands.size() - 1) {
      down_poss.push_back(i);
    }
  }
  int height = down_poss.size();
  int range = max(2, min((int)(8 * temperature + 3), height - 1));
  int start = random.next(0, max(0, height - range - 1));
  int end = start + range;
  int mid = random.next(start + 1, end - 1);
  assert(0 <= start);
  assert(start < mid && mid < end);
  assert(end < (int)down_poss.size());
  return ChangeNode(game, initial_state, commands, loop_count, down_poss[start], down_poss[mid], down_poss[end], start_time);
}

string SubmarineAI::ChangeNode(const Game &game, const State& initial_state, const string& commands, int loop_count, int start_pos, int mid_pos, int end_pos, long long start_time) const {
  assert(commands.size() > 0);
  {
    SmallState initial_sstate;
    initial_sstate.Init(initial_state);
    SmallState temp_sstate = initial_sstate;
    SmallState start_sstate;
    string ret = "";
    if (start_pos == 0) {
      start_sstate = temp_sstate;
      if (mid_pos != 0) {
        temp_sstate.Command(game, initial_state, commands[0]);
      }
    } else {
      for (int i = 0; i <= start_pos; i++) {
        temp_sstate.Command(game, initial_state, commands[i]);
      }
      start_sstate = temp_sstate;
      ret += commands.substr(0, start_pos + 1);
    }
    for (int i = start_pos + 1; i < mid_pos; i++) {
      temp_sstate.Command(game, initial_state, commands[i]);
    }
    SmallState mid_sstate = temp_sstate;
    Cell mid_point = mid_sstate.pivot;
    mid_point.x += random.next(-5, 5);
    // int mid_rot = mid_sstate.rot;
    for (int i = mid_pos; i < end_pos; i++) {
      temp_sstate.Command(game, initial_state, commands[i]);
    }
    Cell end_point = temp_sstate.pivot;
    int end_rot = temp_sstate.rot;

    string mid = FindPath(game, initial_state, start_sstate, loop_count, mid_point, end_point, end_rot, start_time);
    // cout << mid_point.x << " " << mid_point.y << " " << mid << endl;
    if (mid.size() == 0) { goto fail; }
    ret += mid;
    // mid_sstate = CalcLastState(game, initial_state, initial_sstate, ret);
    // assert(mid_sstate.pivot == mid_point && mid_sstate.rot == mid_rot);
    // bool success = false;
    // char cs[2] = { 'a', 'l' };
    // for (int i = 0; i < 2; i++) {
    //   char c = cs[i];
    //   SmallState temp_sstate2 = mid_sstate;
    //   CommandResult command_result = temp_sstate2.Command(game, initial_state, c);
    //   if (command_result == MOVE) {
    //     mid_sstate = temp_sstate2;
    //     ret.push_back(c);
    //     success = true;
    //     break;
    //   }
    // }
    // if (!success) { goto fail; }
    // string mid2 = FindPath(game, initial_state, mid_sstate, loop_count, end_point, end_rot, start_time);
    // if (mid2.size() == 0) { goto fail; }
    // ret += mid2;
    temp_sstate = CalcLastState(game, initial_state, initial_sstate, ret);
    assert(temp_sstate.pivot == end_point && temp_sstate.rot == end_rot);
    ret += commands.substr(end_pos, commands.size() - end_pos);
    return ret;
  }
fail:
  return commands;
}

std::string SubmarineAI::FindPath(const Game &game, const State& initial_state, const SmallState& initial_sstate, int , Cell mid_point, Cell end_point, int end_rot, long long start_time) const {
  int base_score = initial_sstate.score;
  priority_queue<ItemAnnealing> Q;
  Q.push(ItemAnnealing(game, initial_state, initial_sstate, "", base_score, end_point));

  const int visited_w = 3 * game.w;
  const int visited_offset_x = game.w;
  const int visited_h = 3 * game.h;
  const int visited_offset_y = game.h;
  vector<bool> visited(visited_h * visited_w * 6, false);
  int mid_point_visited = 0;
  while (!Q.empty()) {
    TleState tle_state = ShouldExitLoop(start_time, time_limit_per_unit_for_annealing);
    if (tle_state != TleState::Green)
      return "";

    const ItemAnnealing item = Q.top(); Q.pop();

    assert(0 <= item.sstate.pivot.x + visited_offset_x);
    assert(item.sstate.pivot.x + visited_offset_x < visited_w);
    assert(0 <= item.sstate.pivot.y + visited_offset_y);
    assert(item.sstate.pivot.y + visited_offset_y < visited_h);
    const int visited_index =
      visited_w * 6 * (item.sstate.pivot.y + visited_offset_y) +
      6 * (item.sstate.pivot.x + visited_offset_x) + item.sstate.rot;

    if (visited[visited_index]) { continue; }
    visited[visited_index] = true;
    if (end_point == item.sstate.pivot && end_rot == item.sstate.rot) {
      return item.commands;
    }
    if (mid_point == item.sstate.pivot) {
      mid_point_visited++;
    }
    if (item.sstate.pivot.y < mid_point.y &&
        mid_point_visited == game.CurrentPeriod(initial_state.source_idx)) {
      // can't go down
      continue;
    }

    for (auto commands : util::command_map) {
      SmallState next_sstate = item.sstate;
      const CommandResult result = next_sstate.Command(game, initial_state, commands[0]);
      assert(result != GAMEOVER && result != CLEAR);
      if (end_point.y < next_sstate.pivot.y) {
        continue;
      } if (result == ERROR || result == LOCK) {
        continue;
      } else if (result == MOVE) {
        if (next_sstate.pivot.y == mid_point.y &&
            next_sstate.pivot.x != mid_point.x &&
            (util::GetCommand(commands[0]) == util::Command::SOUTH_WEST ||
             util::GetCommand(commands[0]) == util::Command::SOUTH_EAST)) {
          continue;
        }
      } else {
        assert(false);
      }
      int max_pos = -1;
      for (auto c : commands) {
        next_sstate.pma_node = item.sstate.pma_node;
        next_sstate.UpdatePowerPMA(game, initial_state, c);
        if (next_sstate.pma_node->pos < max_pos) { continue; }
        max_pos = next_sstate.pma_node->pos;

        const string next_commands = item.commands + string(1, c);
        Cell target_point = next_sstate.pivot.y < mid_point.y ? mid_point : end_point;
        Q.push(ItemAnnealing(game, initial_state, next_sstate, next_commands, base_score, target_point));
      }
    }
  }
  return "";
}

double SubmarineAI::CalcEnergy(const Game& , const SmallState& last_sstate) const {
  return -last_sstate.score;
}

double SubmarineAI::CalcProbability(double energy, double next_energy, double temperature) const {
  if (next_energy < energy) { return 1; }
  return exp((energy - next_energy) / 2.0) * temperature;
}

string SubmarineAI::Run(const Game& game) {
  string solution;
  State state;
  state.Init(game);

  cerr << "AI: submarine" << endl;

  long long time_limit_per_unit = time_keeper->TimeLimitForTheSeed() / (game.source_seq.size() + 1);
  time_limit_per_unit_for_initial_search = time_limit_per_unit * 0.5;
  time_limit_per_unit_for_annealing = time_limit_per_unit * 0.5;

  for (int loop_count = 0; ; ++loop_count) {
    auto p = Step(game, state, loop_count);
    int max_score = p.first;

    // modify solution by using best_commands
    if (max_score == -1) { break; }
    const string best_commands = Annealing(game, state, p.second, loop_count);

    for (int i = 0; i < (int)best_commands.size(); i++) {
      CommandResult result = state.Command(game, best_commands[i]);
      assert(i == (int)best_commands.size() - 1 || result == MOVE);
    }
    solution += best_commands;
  }

  std::cerr << state.score << endl;
  return solution;
}

SmallState SubmarineAI::CalcLastState(const Game& game, const State& initial_state, const SmallState& sstate, const std::string& commands) const {
  SmallState last_sstate = sstate;
  for (int i = 0; i < (int)commands.size(); i++) {
    CommandResult result = last_sstate.Command(game, initial_state, commands[i]);
    assert(i == (int)commands.size() - 1 || result == MOVE);
  }
  return last_sstate;
}
