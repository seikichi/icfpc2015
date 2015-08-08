#pragma once

#include <fstream>
#include <string>

#include "game.h"
#include "ai.h"

struct EchoAI : public AI {
  std::string Run(const Game& game) {
    char buf[64];
    sprintf(buf, "solutions/%d.txt", game.problem_id);
    std::ifstream ifs(buf);

    if (!ifs.is_open()) return "";

    std::string nop, commands;
    std::getline(ifs, nop);
    std::getline(ifs, commands);
    return commands;
  }
};
