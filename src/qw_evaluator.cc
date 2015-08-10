#include <algorithm>
#include <iostream>
#include <sstream>
#include <queue>
#include <cassert>
#include <cmath>
#include <sys/time.h>
#include <set>
#include "qw_evaluator.h"
using namespace std;

namespace {
double calculateDistanceFromCenter(const Game& game, const vector<pair<int,int> >& current_cell_positions) {
  // units should go to both sides (distance from center)
  int average_x = 0;
  for (const auto& position : current_cell_positions) {
    average_x += position.first;
  }
  return fabs((game.w / 2.0) - (average_x / current_cell_positions.size())) * 1.3;
}

//  bool isWallOrFilled(const Game& game, const State& initial_state, const pair<int,int>& position) {
//    if (position.first < 0 || position.first >= game.w || position.second < 0 || position.second >= game.h){
//      return true;
//    } else if (initial_state.board[Cell(position.first, position.second).Lin(game.w)]){ 
//      return true;
//    }
//    return false;
//  }

int isInUnit(const vector<pair<int,int> >& current_cell_positions, const pair<int,int>& position) {
  for (auto& cell_position : current_cell_positions) {
    if (position == cell_position) {
      return true;
    }
  }
  return false;
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

double calcAverageCellPositionY(const vector<pair<int,int> >& current_cell_positions) {
  int total = 0;
  for(auto& position : current_cell_positions) {
    total += position.second;
  }
  return total / (double) current_cell_positions.size();
}

double calculateLineFilledRatio(const Game& game, const vector<pair<int,int> >& current_cell_positions, const State& initial_state) {
  vector<int> target_y;
  for (const auto& position : current_cell_positions) {
    target_y.push_back(position.second);
  }
  sort(target_y.begin(), target_y.end());
  auto it = unique(target_y.begin(), target_y.end());
  target_y.erase(it, target_y.end());

  int count = 0;
  for (const auto target : target_y) {
    for (int i=0; i<game.w; i++) {
      if (isInUnit(current_cell_positions, make_pair(i,target))) {
        ++count;
      } else if (initial_state.board[Cell(i, target).Lin(game.w)]) {
        ++count;
      }
    }
  }
  double filled_ratio =  count / (double)(target_y.size() * game.w);
  double offset = 1.0;
  return (1 - offset) + filled_ratio * offset;
}
}  // namespace


int QwEvaluator::evaluate(
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
  const Unit& unit = game.CurrentUnit(initial_state.source_idx, sstate.rot);
  vector<pair<int,int> > current_cell_positions;
  for (const auto& cell : unit.cells) {
    Cell current_cell = cell.TranslateAdd(sstate.pivot);
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
  
  int base = 1000000000;
  double average_y = calcAverageCellPositionY(current_cell_positions);
  double ave_y_ratio = 1.0 + fabs(average_y - game.h) * 10 / (double)game.h;
  if (average_y < game.h)
      ave_y_ratio = 1.0 / ave_y_ratio;
  double pivot_members_ratio = 1.0 + fabs(average_y - sstate.pivot.y) / (double)average_y;
  if (average_y > sstate.pivot.y)
      pivot_members_ratio = 1.0 / pivot_members_ratio;
  double filled_neighbors_ratio = 1.5 * calculateFilledNeighbors(game, initial_state, neighbors) / (double) neighbors.size();
  double distance_from_center_ratio = 1.0 + (calculateDistanceFromCenter(game, current_cell_positions) / (game.w / 2.0));
  double point_up_ratio = (next_sstate.score_move + 1) / (double) (sstate.score_move + 1);
  double line_filled_ratio = calculateLineFilledRatio(game, current_cell_positions, initial_state);
  return (int)(base * pivot_members_ratio * point_up_ratio * ave_y_ratio * filled_neighbors_ratio * distance_from_center_ratio * line_filled_ratio);
  
}
