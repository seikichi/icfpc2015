#pragma once

#include "ai.h"

struct ChickAI : public AI
{
	char Step(const Game& game, const State& state);
};