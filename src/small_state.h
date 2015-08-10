#pragma once
#include "game.h"

struct SmallState {
  Cell pivot;  // pivot location
  int rot;
  int score;
  int score_move;  // move_score + line_score
  int score_power; // score by power-of-phrase
  util::Node* pma_node;  // for power-of-phrase check
  util::AcceptIndex used_power;  // set of used power

  void Init(const State &state);
  CommandResult Command(const Game& g, const State& state, char c);
  CommandResult UpdateVisitedAndLock(const Game& g, const State& state, Cell move, char c);
  CommandResult UpdateRotAndLock(const Game& g, const State& state, int dir, char c);
  void Lock(const Game& g, const State& state);
  bool IsLock(const Game& g, const State& state);
  void UpdatePowerPMA(const Game& g, const State& state, char c);
};
