#include "submarineai.h"
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
  State state;
  string commands;
  int priority;
  Item(const Game& game, const State& state, const string& commands)
    : state(state), commands(commands), priority(0) {
    const Unit& unit = game.CurrentUnit(state.source_idx);
    for (const auto& cell : unit.cells) {
        Cell c = cell.Rotate(Cell(0, 0), state.rot).TranslateAdd(state.pivot);
        priority = max((int)(fabs(c.x - game.w/2.0) + (double)game.w * c.y / game.h), priority);
    }
  }

  bool operator<(const Item& rhs) const {
    return priority < rhs.priority;
  }
};

struct ItemAnnealing {
  State state;
  string commands;
  int priority;
  ItemAnnealing(const Game& game, const State& state, const string& commands, int base_score, const Cell& target_cell)
    : state(state), commands(commands), priority(0) {
      int dist = abs(target_cell.x - state.pivot.x) + abs(target_cell.y - state.pivot.y);
      priority = (state.score - base_score) * 100 + state.pma_node->pos * 10 - dist;
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

int calculateDistanceFromCenter(const Game& game, const vector<pair<int,int> >& current_cell_positions) {
  // units should go to both sides (distance from center)
  int average_x = 0;
  for (const auto& position : current_cell_positions) {
    average_x += position.first;
  }
  return fabs((game.w / 2) - (int)(average_x / current_cell_positions.size()));
}

bool isWallOrFilled(const Game& game, const State& state, const pair<int,int>& position) {
  if (position.first < 0 || position.first >= game.w || position.second < 0 || position.second >= game.h){
    return true;
  } else if (state.board[Cell(position.first, position.second).Lin(game.w)]){ 
    return true;
  }
  return false;
}

int isInUnit(const vector<pair<int,int> >& current_cell_positions, const pair<int,int>& position) {
  for (auto& cell_position : current_cell_positions) {
    if (position == cell_position) {
      return true;
    }
  }
  return false;
}

// if the result makes hole whose size is within "threshold", get the score "penalty"
double avoidHole(const Game& game, 
              const State& state,
              const vector<pair<int,int> >& current_cell_positions,
              const vector<pair<int,int> >& neighbors,
              const vector<vector<pair<int,int> > >& directions,
              const double penalty,
              const double normal,
              const int threshold) {
  // how many cells from neighbor? (bfs)
  for (const auto& neighbor : neighbors) {
    set<pair<int,int> > not_filled_cell_positions;
    queue<pair<int,int> > que;

    if (isInUnit(current_cell_positions, neighbor) || isWallOrFilled(game, state, neighbor)) {
      continue;
    }
    que.push(neighbor);

    while (!que.empty() && (int)not_filled_cell_positions.size() <= threshold) {
      auto& target = que.front(); que.pop();
      not_filled_cell_positions.insert(target);

      for(const auto& direction : directions[target.second % 2]) {
        pair<int, int> next(target.first + direction.first, target.second + direction.second);
        if (not_filled_cell_positions.count(next) == 1) {
          continue;
        }
        if (!isInUnit(current_cell_positions, next) && !isWallOrFilled(game, state, next)) {
          que.push(next);
        }
      }
    }
    if ((int)not_filled_cell_positions.size() <= threshold) {
      return penalty;
    }
  }
  return normal;
}


int calculateFilledNeighbors(const Game& game, const State& state, const vector<pair<int,int> >& neighbors) {
  int filled_neighbor = 0;
  for(const auto& neighbor : neighbors) {
      if (neighbor.first < 0 || neighbor.first >= game.w || neighbor.second >= game.h){
        ++filled_neighbor;
      } else if (state.board[Cell(neighbor.first, neighbor.second).Lin(game.w)]) {
        ++filled_neighbor;
      }
  }
  return filled_neighbor;
}

int calcSumCellPositionY(const vector<pair<int,int> >& current_cell_positions) {
  int total = 0;
  for(auto& position : current_cell_positions) {
    total += position.second;
  }
  return total;
}

int evaluateScore(const Game& game, const State& state, const State& next_state) {
  // units should go to the place which has many neighbors (number_of_neighbors)
  vector< vector<pair<int, int> > > directions = {
    {
      make_pair(-1,-1), make_pair( 0,-1), make_pair(-1, 0),
      make_pair( 1, 0), make_pair(-1, 1), make_pair( 0, 1)
    }, {
      make_pair( 0,-1), make_pair( 1,-1), make_pair(-1, 0),
      make_pair( 1, 0), make_pair( 0, 1), make_pair( 1, 1)
    }
  };

  // get current status
  const Unit& unit = game.CurrentUnit(state.source_idx);
  vector<pair<int,int> > current_cell_positions;
  for (const auto& cell : unit.cells) {
    Cell current_cell = cell.Rotate(Cell(0,0), state.rot).TranslateAdd(state.pivot);
    current_cell_positions.push_back(make_pair(current_cell.x, current_cell.y));
  }

  // calculate neighbor cells
  vector<pair<int,int> > neighbors;
  for (const auto& position : current_cell_positions) {
    for (const auto& direction : directions[position.second % 2]) {   
      pair<int, int> neighbor(position.first + direction.first, position.second + direction.second);
      if (neighbor.second < 0) {
        continue;
      } else {
        neighbors.push_back(neighbor);
      }
    }
  }
  sort(neighbors.begin(), neighbors.end());
  auto it = unique(neighbors.begin(), neighbors.end());
  neighbors.erase(it, neighbors.end());
  
  double hole_penalty_base = 1.0;
  int hole_size_threshold = 1;
  
  /*
  int ave_y = calcSumCellPositionY(current_cell_positions) / current_cell_positions.size();
  int number_of_neighbors = calculateFilledNeighbors(game, state, neighbors);
  int distance_from_center = 0; //calculateDistanceFromCenter(game, current_cell_positions);
  int hole_penalty = 0; //avoidHole(game, state, current_cell_positions, neighbors, directions, hole_penalty_base, 0, hole_size_threshold);
  return ave_y + next_state.score + number_of_neighbors + distance_from_center + hole_penalty;
  */
  
  int base = 1000000000;
  double ave_y_ratio = 1.1 * calcSumCellPositionY(current_cell_positions) / (double)(current_cell_positions.size() * game.h);
  double filled_neighbors_ratio = calculateFilledNeighbors(game, state, neighbors) / (double) neighbors.size();
  double distance_from_center = 1; //calculateDistanceFromCenter(game, current_cell_positions);
  double hole_penalty = 1; //avoidHole(game, state, current_cell_positions, neighbors, directions, hole_penalty_base, 1, hole_size_threshold);
  double point_up_ratio = (next_state.score + 1) / (double) (state.score + 1);
  return (int)(base * point_up_ratio * ave_y_ratio * filled_neighbors_ratio * distance_from_center * hole_penalty);
}
};


TleState SubmarineAI::ShouldExitLoop(long long unit_start_time, long long time_limit_per_unit) const {
  long long now = getTime();
  if (now - unit_start_time >= time_limit_per_unit)
    return TleState::Yellow;
  if (now - game_start_time >= time_limit / 2)
    return TleState::Red;
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
  Q.push(Item(game, initial_state, ""));

  long long start_time = getTime();
  while (!Q.empty()) {
    TleState tle_state = ShouldExitLoop(start_time, time_limit_per_unit_for_initial_search);
    if (tle_state == TleState::Yellow && max_score != -1)
      break;
    else if (tle_state == TleState::Red)
      return make_pair(-1, "");

    const Item item = Q.top(); Q.pop();

    assert(0 <= item.state.pivot.x + visited_offset_x);
    assert(item.state.pivot.x + visited_offset_x < visited_w);
    assert(0 <= item.state.pivot.y + visited_offset_y);
    assert(item.state.pivot.y + visited_offset_y < visited_h);
    const int visited_index =
      visited_w * 6 * (item.state.pivot.y + visited_offset_y) +
      6 * (item.state.pivot.x + visited_offset_x) + item.state.rot;

    if (visited[visited_index]) { continue; }
    visited[visited_index] = true;

    const vector<char> commands = {'!', 'e', 'i', ' ', 'd', 'k'};
    for (auto c : commands) {
      State next_state = item.state;
      const CommandResult result = next_state.Command(game, c);
      const string next_commands = item.commands + string(1, c);

      assert(result != GAMEOVER && result != CLEAR);
      if (result == ERROR) {
        continue;
      } else if (result == LOCK) {
        const int score = evaluateScore(game, item.state, next_state);
        if (score > max_score) {
          max_score = score;
          best_commands = next_commands;
        }
      } else if (result == MOVE) {
        Q.push(Item(game, next_state, next_commands));
      } else {
        assert(false);
      }
    }
  }

  cerr << "Loop " << loop_count << ": time=" << getTime() - start_time << " usec, total=" << getTime() - game_start_time << " usec" << endl;

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
  State initial_last_state = CalcLastState(game, initial_state, initial_answer);
  double energy = CalcEnergy(game, initial_last_state);
  pair<double, string> best_answer = make_pair(energy, initial_answer);
  int counter = 0;
  long long start_time = getTime();
  long long end_time = time_limit_per_unit_for_annealing + start_time;
  while (true) {
    TleState tle_state = ShouldExitLoop(start_time, time_limit_per_unit_for_annealing);
    if (tle_state == TleState::Yellow || tle_state == TleState::Red) { break; }
    long long current_time = getTime();
    double rest_time = (double)(end_time - current_time) / (double)(end_time - start_time) + 1e-8;
    string next_commands = ChangePath(game, initial_state, commands, loop_count, rest_time, start_time);
    State next_last_state = CalcLastState(game, initial_state, next_commands);
    assert(next_last_state.pivot == initial_last_state.pivot);
    assert(next_last_state.rot == initial_last_state.rot);
    double next_energy = CalcEnergy(game, next_last_state);
    double probability = CalcProbability(energy, next_energy,  rest_time + 1e-8);
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
    if (c == util::Command::SOUTH_WEST || c == util::Command::SOUTH_EAST) {
      down_poss.push_back(i);
    }
  }
  int height = down_poss.size();
  int range = min((int)(8 * temperature + 2), height - 1);
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
    State temp_state = initial_state;
    State start_state;
    string ret = "";
    if (start_pos == 0) {
      start_state = temp_state;
      if (mid_pos != 0) {
        temp_state.Command(game, commands[0]);
      }
    } else {
      for (int i = 0; i <= start_pos; i++) {
        temp_state.Command(game, commands[i]);
      }
      start_state = temp_state;
      ret += commands.substr(0, start_pos + 1);
    }
    for (int i = start_pos + 1; i < mid_pos; i++) {
      temp_state.Command(game, commands[i]);
    }
    State mid_state = temp_state;
    Cell mid_point = mid_state.pivot;
    mid_point.x += random.next(-5, 5);
    int mid_rot = mid_state.rot;
    for (int i = mid_pos; i < end_pos; i++) {
      temp_state.Command(game, commands[i]);
    }
    Cell end_point = temp_state.pivot;
    int end_rot = temp_state.rot;

    string mid1 = FindPath(game, start_state, loop_count, mid_point, mid_rot, start_time);
    if (mid1.size() == 0) { goto fail; }
    ret += mid1;
    mid_state = CalcLastState(game, initial_state, ret);
    assert(mid_state.pivot == mid_point && mid_state.rot == mid_rot);
    bool success = false;
    char cs[2] = { 'a', 'l' };
    for (int i = 0; i < 2; i++) {
      char c = cs[i];
      State temp_state2 = mid_state;
      CommandResult command_result = temp_state2.Command(game, c);
      if (command_result == MOVE) {
        mid_state = temp_state2;
        ret.push_back(c);
        success = true;
        break;
      }
    }
    if (!success) { goto fail; }
    string mid2 = FindPath(game, mid_state, loop_count, end_point, end_rot, start_time);
    if (mid2.size() == 0) { goto fail; }
    ret += mid2;
    temp_state = CalcLastState(game, initial_state, ret);
    assert(temp_state.pivot == end_point && temp_state.rot == end_rot);
    ret += commands.substr(end_pos, commands.size() - end_pos);
    return ret;
  }
fail:
  return commands;
}

std::string SubmarineAI::FindPath(const Game &game, const State& initial_state, int loop_count, Cell end_point, int end_rot, long long start_time) const {
  int base_score = initial_state.score;
  priority_queue<ItemAnnealing> Q;
  Q.push(ItemAnnealing(game, initial_state, "", base_score, end_point));

  const int visited_w = 3 * game.w;
  const int visited_offset_x = game.w;
  const int visited_h = 3 * game.h;
  const int visited_offset_y = game.h;
  vector<bool> visited(visited_h * visited_w * 6, false);
  while (!Q.empty()) {
    TleState tle_state = ShouldExitLoop(start_time, time_limit_per_unit_for_annealing);
    if (tle_state != TleState::Green)
      return "";

    const ItemAnnealing item = Q.top(); Q.pop();

    assert(0 <= item.state.pivot.x + visited_offset_x);
    assert(item.state.pivot.x + visited_offset_x < visited_w);
    assert(0 <= item.state.pivot.y + visited_offset_y);
    assert(item.state.pivot.y + visited_offset_y < visited_h);
    const int visited_index =
      visited_w * 6 * (item.state.pivot.y + visited_offset_y) +
      6 * (item.state.pivot.x + visited_offset_x) + item.state.rot;

    if (visited[visited_index]) { continue; }
    visited[visited_index] = true;
    if (end_point == item.state.pivot && end_rot == item.state.rot) {
      return item.commands;
    }

    const string commands = { "p'!.03bcefy2aghij4lmno 5dqrvz1kstuwx" };
    for (auto c : commands) {
      State next_state = item.state;
      const CommandResult result = next_state.Command(game, c);
      const string next_commands = item.commands + string(1, c);

      assert(result != GAMEOVER && result != CLEAR);
      if (end_point.y < next_state.pivot.y) {
        continue;
      } if (result == ERROR || result == LOCK) {
        continue;
      } else if (result == MOVE) {
        Q.push(ItemAnnealing(game, next_state, next_commands, base_score, end_point));
      } else {
        assert(false);
      }
    }
  }
  return "";
}

double SubmarineAI::CalcEnergy(const Game& game, const State& last_state) const {
  return -last_state.score;
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

  time_limit = (long long)time_limit_seconds * 1000000;
  long long time_limit_per_unit = 4 * time_limit / (5 * game.num_source_seeds * game.source_seq.size());
  time_limit_per_unit_for_initial_search = time_limit_per_unit * 0.5;
  time_limit_per_unit_for_annealing = time_limit_per_unit * 0.5;
  game_start_time = getTime();

  for (int loop_count = 0; ; ++loop_count) {
    auto p = Step(game, state, loop_count);
    int max_score = p.first;

    // modify solution by using best_commands
    if (max_score == -1) { break; }
    const string best_commands = Annealing(game, state, p.second, loop_count);
    state = CalcLastState(game, state, best_commands);
    solution += best_commands;
  }

  std::cerr << state.score << endl;
  return solution;
}

State SubmarineAI::CalcLastState(const Game& game, const State& state, const std::string& commands) const {
  State last_state = state;
  for (int i = 0; i < (int)commands.size(); i++) {
    CommandResult result = last_state.Command(game, commands[i]);
    assert(i == (int)commands.size() - 1 || result == MOVE);
  }
  return last_state;
}
