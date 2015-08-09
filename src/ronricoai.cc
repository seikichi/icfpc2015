#include "ronricoai.h"
#include <iostream>
#include <sstream>
#include <queue>
#include <cassert>
#include <cmath>
#include <utility>
#include <sys/time.h>
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

// int evaluateScore_0815_1917(const Game&, const State& state, const State&) {
//   return state.pivot.y;
// }
int calculateDistanceFromCenter(const Game& game, const vector<Cell>& current_cells) {
  // units should go to both sides (distance from center)
  int average_x = 0;
  for (const auto& cell : current_cells) {
    average_x += cell.x;
  }
  return fabs((game.w / 2) - (int)(average_x / current_cells.size()));
}

int calculateNeighbors(const Game& game,  const State& state, const vector<Cell>& current_cells) {
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
  
  vector<pair<int, int> > filled_neighbors;
  for (const auto& cell : current_cells) {
    for (const auto& direction : directions[cell.y % 2]) {
      pair<int, int> neighbor(cell.x + direction.first, cell.y + direction.second);
      if (neighbor.second < 0) {
        continue;
      } else if (neighbor.first < 0 || neighbor.first >= game.w || neighbor.second >= game.h){
        filled_neighbors.push_back(neighbor);
      } else if (state.board[Cell(neighbor.first, neighbor.second).Lin(game.w)]) {
        filled_neighbors.push_back(neighbor);
      }
    }
  }
  sort(filled_neighbors.begin(), filled_neighbors.end());
  auto it = unique(filled_neighbors.begin(), filled_neighbors.end());
  return distance(filled_neighbors.begin(), it);
}

int evaluateScore(const Game& game,  const State& state, const State& next_state) {

  const Unit& unit = game.CurrentUnit(state.source_idx);
  vector<Cell> current_cells;
  for (const auto& cell : unit.cells) {
    current_cells.push_back(cell.Rotate(Cell(0,0), state.rot).TranslateAdd(state.pivot));
  }
  
  int number_of_neighbors = calculateNeighbors(game, state, current_cells);
  int distance_from_center = calculateDistanceFromCenter(game, current_cells) * 0;
  return state.pivot.y + next_state.score + number_of_neighbors + distance_from_center;
}

long long getTime() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (long long)tv.tv_sec * 1000000 + (long long)tv.tv_usec;
}

};

string RonricoAI::Run(const Game& game) {
  string solution;
  State state;
  state.Init(game);

  long long time_limit = (long long)time_limit_seconds * 1000000;
  long long time_limit_per_unit = time_limit / (2 * game.num_source_seeds * game.source_seq.size());
  long long game_start_time = getTime();

  for (int loop_count = 0; ; ++loop_count) {
    // 1ユニットごとにループ
    const int visited_w = 3 * game.w;
    const int visited_offset_x = game.w;
    const int visited_h = 3 * game.h;
    const int visited_offset_y = game.h;
    vector<bool> visited(visited_h * visited_w * 6, false);

    if (state.IsClear(game) || state.IsGameOver(game)) { break; }

    int max_score = -1;
    string best_commands = "";
    priority_queue<Item> Q;
    Q.push(Item(game, state, ""));

    long long start_time = getTime();
    for (unsigned loop_count = 0; !Q.empty(); ++loop_count) {
      long long now = getTime();
      if (now - start_time >= time_limit_per_unit && max_score != -1)
          break;
      if (now - game_start_time >= time_limit / 2)
          goto finish;

      const Item item = Q.top(); Q.pop();

      assert(0 <= state.pivot.x + visited_offset_x);
      assert(state.pivot.x + visited_offset_x < visited_w);
      assert(0 <= state.pivot.y + visited_offset_y);
      assert(state.pivot.y + visited_offset_y < visited_h);
      const int visited_index =
        visited_w * 6 * (item.state.pivot.y + visited_offset_y) +
        6 * (item.state.pivot.x + visited_offset_x) + item.state.rot;

      if (visited[visited_index]) { continue; }
      visited[visited_index] = true;

      //const vector<char> commands = {'!', 'e', 'i', ' ', 'd', 'k'};
      const vector<char> commands = {'p', 'b', 'a', 'l', 'd', 'k'};
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

    // modify solution by using best_commands
    if (max_score == -1) { break; }
    for (char c : best_commands) { state.Command(game, c); }
    solution += best_commands;
  }

finish:
  std::cerr << state.score << endl;
  return solution;
}
