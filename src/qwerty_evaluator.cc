#include <algorithm>
#include <iostream>
#include <sstream>
#include <queue>
#include <cassert>
#include <cmath>
#include <sys/time.h>
#include <set>
#include "qwerty_evaluator.h"
using namespace std;

namespace {

int calculateDistanceFromCenter(const Game& game, const vector<pair<int,int> >& current_cell_positions) {
  // units should go to both sides (distance from center)
  int average_x = 0;
  for (const auto& position : current_cell_positions) {
    average_x += position.first;
  }
  return fabs((game.w / 2) - (int)(average_x / current_cell_positions.size()));
}

bool isWallOrFilled(const Game& game, const State& initial_state, const pair<int,int>& position) {
  if (position.first < 0 || position.first >= game.w || position.second < 0 || position.second >= game.h){
    return true;
  } else if (initial_state.board[Cell(position.first, position.second).Lin(game.w)]){ 
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
              const State& initial_state,
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

    if (isInUnit(current_cell_positions, neighbor) || isWallOrFilled(game, initial_state, neighbor)) {
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
        if (!isInUnit(current_cell_positions, next) && !isWallOrFilled(game, initial_state, next)) {
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


int calculateFilledNeighbors(const Game& game, const State& initial_state, const vector<pair<int,int> >& neighbors) {
  int filled_neighbor = 0;
  for(const auto& neighbor : neighbors) {
      if (neighbor.first < 0 || neighbor.first >= game.w || neighbor.second >= game.h){
        ++filled_neighbor;
      } else if (initial_state.board[Cell(neighbor.first, neighbor.second).Lin(game.w)]) {
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

}  // namespace


int QwertyEvaluator::evaluate(
    const Game& game,
    const State& initial_state,
    const SmallState& sstate,
    const SmallState& next_sstate) {
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
  const Unit& unit = game.CurrentUnit(initial_state.source_idx);
  vector<pair<int,int> > current_cell_positions;
  for (const auto& cell : unit.cells) {
    Cell current_cell = cell.Rotate(Cell(0,0), sstate.rot).TranslateAdd(sstate.pivot);
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
  
  // double hole_penalty_base = 1.0;
  // int hole_size_threshold = 1;
  
  /*
  int ave_y = calcSumCellPositionY(current_cell_positions) / current_cell_positions.size();
  int number_of_neighbors = calculateFilledNeighbors(game, initial_state, neighbors);
  int distance_from_center = 0; //calculateDistanceFromCenter(game, current_cell_positions);
  int hole_penalty = 0; //avoidHole(game, initial_state, current_cell_positions, neighbors, directions, hole_penalty_base, 0, hole_size_threshold);
  return ave_y + next_sstate.score + number_of_neighbors + distance_from_center + hole_penalty;
  */
  
  int base = 1000000000;
  double ave_y_ratio = 1.1 * calcSumCellPositionY(current_cell_positions) / (double)(current_cell_positions.size() * game.h);
  double filled_neighbors_ratio = calculateFilledNeighbors(game, initial_state, neighbors) / (double) neighbors.size();
  double distance_from_center = 1; //calculateDistanceFromCenter(game, current_cell_positions);
  double hole_penalty = 1; //avoidHole(game, initial_state, current_cell_positions, neighbors, directions, hole_penalty_base, 1, hole_size_threshold);
  double point_up_ratio = (next_sstate.score_move + 1) / (double) (sstate.score_move + 1);
  return (int)(base * point_up_ratio * ave_y_ratio * filled_neighbors_ratio * distance_from_center * hole_penalty);
}
