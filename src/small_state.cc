#include "small_state.h"

#include <assert.h>
#include <string>
#include <vector>

using namespace std;

void SmallState::Init(const State &state) {
  pivot = state.pivot;
  rot = state.rot;
  score = state.score;
  score_move = state.score_move;
  score_power = state.score_power;
  pma_node = state.pma_node;
  used_power = state.used_power;
}

CommandResult SmallState::Command(const Game& g, const State& state, char c) {
  assert(!state.IsClear(g));
  assert(!state.IsGameOver(g));

  util::Command command = util::GetCommand(c);
  if (command == util::Command::SIZE) {
    cerr << "AHOKA:Invalid command c=" << c << endl;
    exit(1);
  }

  if (command == util::Command::WEST) {
    return UpdateVisitedAndLock(g, state, Cell(-1, 0), c);
  }

  if (command == util::Command::EAST) {
    return UpdateVisitedAndLock(g, state, Cell(+1, 0), c);
  }

  if (command == util::Command::SOUTH_WEST) {
    return UpdateVisitedAndLock(g, state, Cell(-1, 1), c);
  }

  if (command == util::Command::SOUTH_EAST) {
    return UpdateVisitedAndLock(g, state, Cell(0, 1), c);
  }

  if (command == util::Command::CLOCKWISE) {
    return UpdateRotAndLock(g, state, -1, c);
  }

  if (command == util::Command::COUNTER_CLOCKSISE) {
    return UpdateRotAndLock(g, state, +1, c);
  }

  if (command == util::Command::IGNORE) {
    // just ignore
    return MOVE;
  }

  cerr << "?????" << endl;
  exit(1);
  return ERROR;
}

CommandResult SmallState::UpdateVisitedAndLock(const Game& g, const State& state, Cell move, char c) {
  assert(move.y >= 0);
  pivot = pivot.TranslateAdd(move);
  UpdatePowerPMA(g, state, c);

  // Check Lock
  const auto& unit = g.CurrentUnit(state.source_idx, rot);
  for (const auto& cell : unit.cells) {
    Cell c = cell.TranslateAdd(pivot);
    if (c.x < 0 || c.y < 0 || c.x >= g.w || c.y >= g.h || state.board[c.Lin(g.w)]) {
      // The unit must be locked, revert the pivot and terminate
      pivot = pivot.TranslateSub(move);
      Lock(g, state);
      return LOCK;
    }
  }

  return MOVE;
}

CommandResult SmallState::UpdateRotAndLock(const Game& g, const State& state, int dir, char c) {
  int p = g.CurrentPeriod(state.source_idx);
  rot = (rot + dir + p) % p;
  UpdatePowerPMA(g, state, c);

  // NOTE: Copy & Paste is good
  const auto& unit = g.CurrentUnit(state.source_idx, rot);
  for (const auto& cell : unit.cells) {
    Cell c = cell.TranslateAdd(pivot);
    if (c.x < 0 || c.y < 0 || c.x >= g.w || c.y >= g.h || state.board[c.Lin(g.w)]) {
      // Revert and lock
      rot = (rot - dir + p) % p;
      Lock(g, state);
      return LOCK;
    }
  }

  return MOVE;
}

void SmallState::UpdatePowerPMA(const Game& g, const State& , char c) {
  auto accept_index = g.power_pma.UpdateNode(c, pma_node);

  // Update score
  int now_power = 0;
  for (int i = 0; i < (int)accept_index.size(); ++i) {
    if (accept_index[i]) {
      if (!used_power[i]) {
        now_power += 300;  // Bonus
        used_power[i] = 1;
      }
      now_power += 2 * g.power_len[i];
    }
  }
  score += now_power;
  score_power = now_power;
}

void SmallState::Lock(const Game& g, const State& state) {
  vector<short> fill_count = state.fill_count;
  const auto& unit = g.CurrentUnit(state.source_idx, rot);
  for (const auto& cell : unit.cells) {
    Cell c = cell.TranslateAdd(pivot);
    const int idx = c.Lin(g.w);
    assert(state.board[idx] == 0);
    fill_count[c.y]++;
  }

  // Line deletion
  int ls = 0;
  for (int y = 0; y < g.h; ++y) {
    if (fill_count[y] == g.w) {
      ls++;
    }
  }

  // Compute and update score
  int size = (int)unit.cells.size();
  int points = size + 100 * (1 + ls) * ls / 2;
  int line_bonus = (state.ls_old > 1) ? (state.ls_old - 1) * points / 10 : 0;
  score += points + line_bonus;
  score_move += points + line_bonus;
}
