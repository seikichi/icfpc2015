#include <algorithm>
#include <iostream>
#include <sstream>
#include <queue>
#include <cassert>
#include <cmath>
#include <sys/time.h>
#include <set>
#include "kichi_evaluator.h"
using namespace std;

int KichiEvaluator::evaluate(
    const Game& game,
    const State& initial_state,
    const SmallState& sstate,
    const SmallState& next_sstate) {

  return next_sstate.pivot.y;
}
