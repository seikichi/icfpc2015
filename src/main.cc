#include "ai.h"
#include "chickai.h"
#include "echo_ai.h"
#include "game.h"
#include "util.h"

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
  util::Init();
  // input
  vector<string> problem_files;
  int time_limit_seconds = 300;
  int memory_limit = 100;
  int cores = 2;
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
      break;
    case 'p':
      phrases_of_power.push_back(optarg);
      break;
    default:
      break;
    }
  }

  TimeKeeper time_keeper;
  time_keeper.Init(problem_files, (long long)time_limit_seconds * 1000000);

  // process
  stringstream ss;
  ss << "[";
  bool first = true;
  for (int problem_id = 0; problem_id < (int)problem_files.size(); ++problem_id) {
    Game game;

    ifstream ifs(problem_files[problem_id].c_str());
    string problem((istreambuf_iterator<char>(ifs)), istreambuf_iterator<char>());

    time_keeper.StartNewProblem(problem_id);

    for (int source_seed_idx = 0; ; ++source_seed_idx) {
      bool ok = game.Init(problem, source_seed_idx);
      if (!ok)
        break;
      time_keeper.StartNewSeed(source_seed_idx);

      if (!first) { ss << ","; }

      pair<int, string> best{-1, "???????"};

      // Fetch result of annotated solution
      {
        auto echo_ai = make_shared<EchoAI>();
        echo_ai->Init(time_keeper);
        string solution = echo_ai->Run(game);

        int eval = Evaluate(game, solution);
        best = max(best, make_pair(eval, solution));
        cerr << "# Annotated score: " << eval << endl;
      }

      // Then, execute specified AI
      {
        auto ai = AI::CreateAI();
        ai->Init(time_keeper);
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
