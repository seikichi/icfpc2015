#include "submarineai.h"
#include <iostream>
#include <sstream>
#include <queue>
#include <cassert>
#include <cmath>
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

int evaluateScore(const Game&, const State& state, const State& next_state) {
  return state.pivot.y + next_state.score;
  // return state.pivot.y;
}

long long getTime() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (long long)tv.tv_sec * 1000000 + (long long)tv.tv_usec;
}

};

string SubmarineAI::Run(const Game& game) {
  string solution;
  State state;
  state.Init(game);

  cerr << "AI: submarine" << endl;

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
