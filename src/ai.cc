#include "ai.h"

#include <iostream>
#include <memory>
#include <sstream>

#include "game.h"
#include "chickai.h"
#include "kichiai.h"

using namespace std;

void AI::Init() {}

shared_ptr<AI> AI::CreateAI() {
  shared_ptr<AI> result = make_shared<KichiAI>();
  return result;
}
