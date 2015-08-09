#include "ai.h"
#include "chickai.h"
#include "echo_ai.h"
#include "game.h"

#include <cstdio>
#include <cstdlib>
#include <unistd.h>

#include <fstream>
#include <memory>
#include <sstream>
#include <streambuf>
#include <string>
#include <vector>

#include "picojson/picojson.h"
using namespace std;

namespace {

int Evaluate(const Game& g, const string commands) {
  State s;
  s.Init(g);

  for (char c : commands) {
    auto res = s.Command(g, c);
    if (res != MOVE && res != LOCK) {
      return 0;
    }
  }

  return s.score;
}

}  // namespace

int main(int argc, char** argv) {
  // input
  vector<string> problem_files;
  int time_limit_seconds = 300;
  int memory_limit;
  int cores;
  vector<string> phrases_of_power;

  int result;
  while ((result = getopt(argc, argv, "f:t:m:c:p:")) != -1) {
    switch (result) {
    case 'f':
      problem_files.push_back(optarg);
      break;
    case 't':
      time_limit_seconds = stoi(optarg);
      break;
    case 'm':
      memory_limit = stoi(optarg);
      break;
    case 'c':
      cores = stoi(optarg);
    case 'p':
      phrases_of_power.push_back(optarg);
      break;
    default:
      break;
    }
  }

  // process
  stringstream ss;
  ss << "[";
  bool first = true;
  for (const auto& problem_file : problem_files) {
    Game game;
    auto source_seed_idx = 0;

    ifstream ifs(problem_file.c_str());
    string problem((istreambuf_iterator<char>(ifs)), istreambuf_iterator<char>());;

    while (game.Init(problem, source_seed_idx++)) {
      if (!first) { ss << ","; }

      pair<int, string> best{-1, "???????"};

      // Fetch result of annotated solution
      {
        auto echo_ai = make_shared<EchoAI>();
        echo_ai->Init(time_limit_seconds);
        string solution = echo_ai->Run(game);

        int eval = Evaluate(game, solution);
        best = max(best, make_pair(eval, solution));
        cerr << "# Annotated score: " << eval << endl;
      }

      // Then, execute specified AI
      {
        auto ai = AI::CreateAI();
        ai->Init(time_limit_seconds);
        auto solution = ai->Run(game);

        best = max(best, make_pair(Evaluate(game, solution),
                                   solution));
      }

      cerr << "best_score: " << best.first << endl;

      auto solution = best.second;
      ss << "{";
      ss << "\"problemId\": " << game.problem_id << ", ";
      ss << "\"seed\": " << game.source_seed << ", ";
      ss << "\"solution\": " << "\"" << solution << "\"";
      ss << "}";
      first = false;
    }
  }
  ss << "]";

  // output
  cout << ss.str() << endl;
  return 0;
}
