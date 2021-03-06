#include "ai.h"
#include "game.h"
#include "visualizer.h"
#include "kichiai.h"
#include "key_input.h"
#include "manual_player.h"
#include "replay.h"
#include "echo_ai.h"
#include "util.h"

#include <cstdio>
#include <unistd.h>

#include <fstream>
#include <memory>
#include <sstream>
#include <streambuf>
#include <string>
#include <vector>

#include <SDL.h>
#include "picojson/picojson.h"
using namespace std;

void OutputScores(const Game &game, const std::string& commands) {
  Replay replay;
  replay.Init(game, commands);
  printf("{\"score\":[");
  bool first = true;
  while (replay.OneCommandStep(game)) {
    if (!first) { printf(","); }
    printf("%d", replay.GetCurrentState().score);
    first = false;
  }
  printf("]}\n");
}

int CalcScore(const Game &game, const std::string& commands) {
  Replay replay;
  replay.Init(game, commands);
  while (replay.OneCommandStep(game)) {;}
  return replay.GetCurrentState().score;
}

void EventLoopAI(const Game& game, const std::string& commands) {
  SDL_Event event;
  double next_frame = SDL_GetTicks();
  int speed = 1;
  double wait = 1000.0 / 60;
  KeyInput keys;
  keys.Init();

  CommandResult command_result = MOVE;
  Replay replay;
  replay.Init(game, commands);
  Visualizer visualizer;
  visualizer.Init(game);
  for (;;) {
    /* すべてのイベントを処理する */
    while (SDL_PollEvent(&event)) {
      /* QUIT イベントが発生するか、ESC キーが押されたら終了する */
      if ((event.type == SDL_QUIT) ||
          (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE)) {
        goto end;
      }
    }
    /* 1秒間に60回Updateされるようにする */
    if (SDL_GetTicks() >= next_frame) {
      keys.Update();
      if (keys.Pushed('x')) {
        speed++;
      } else if (keys.Pushed('z')) {
        speed--;
      } else if (keys.Pushed('d')) {
        speed += 10;
      } else if (keys.Pushed('a')) {
        speed -= 10;
      } else if (keys.Pushed(' ')) {
        replay.Init(game, commands);
      }
      speed = max(speed, 0);
      bool end = false;
      int s = speed + (keys.Pushed('c') ? 1 : 0);
      for (int i = 0; i < s; i++)  {
        end = !replay.OneCommandStep(game);
        if (end) { speed = 0; break; }
      }

      visualizer.BeginDraw();
      visualizer.DrawGameState(game, replay.GetCurrentState());
      visualizer.DrawCommandResult(game, command_result);
      visualizer.DrawText(196, visualizer.GetBoardHeight(game) + 8, "Speed x %d", speed);
      visualizer.EndDraw();
      next_frame += wait;
      // SDL_Delay(1);
    }
    SDL_Delay(5);
  }
end:;
}


void EventLoopManual(const Game& game) {
  SDL_Event event;
  double next_frame = SDL_GetTicks();
  double wait = 1000.0 / 60;
  KeyInput keys;
  keys.Init();

  CommandResult command_result = MOVE;

  ManualPlayer player;
  player.Init(game);

  Visualizer visualizer;
  visualizer.Init(game);
  for (;;) {
    /* すべてのイベントを処理する */
    while (SDL_PollEvent(&event)) {
      /* QUIT イベントが発生するか、ESC キーが押されたら終了する */
      if ((event.type == SDL_QUIT) ||
          (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE)) {
        goto end;
      }
    }
    /* 1秒間に60回Updateされるようにする */
    if (SDL_GetTicks() >= next_frame) {
      keys.Update();
      char c = 0;
      for (auto key : keys.target_keys) {
        if (keys.Pushed(key.first)) {
          c = key.first;
        }
      }
      if (c != 0) {
        command_result = player.Move(game, c);
      }

      visualizer.BeginDraw();
      visualizer.DrawGameState(game, player.GetCurrentState());
      visualizer.DrawCommandResult(game, command_result);
      visualizer.EndDraw();
      next_frame += wait;
      // SDL_Delay(1);
    }
    SDL_Delay(5);
  }
end:
  EchoAI ai;
  string commands = ai.Run(game);
  int prev_score = CalcScore(game, commands);
  int score = player.GetCurrentState().score;
  if (score > prev_score) {
    char filename[1000];
    snprintf(filename, 999, "./solutions/%d.txt", game.problem_id);
    stringstream ss;
    ss << game.problem_id << " " << game.source_seed << endl << player.GetCommands() << endl;
    FILE *fp = fopen(filename, "w");
    if (fp == nullptr) { return; }
    fprintf(fp, "%s", ss.str().c_str());
    fclose(fp);
  }
  cout << prev_score << " " << score << endl;
  // ss << "[";
  // ss << "{";
  // ss << "\"problemId\": " << game.problem_id << ", ";
  // ss << "\"seed\": " << game.source_seed << ", ";
  // ss << "\"tag\": " << "\"kyoto ni modoritai\"" << ", ";
  // ss << "\"solution\": " << "\"" << player.GetCommands() << "\"";
  // ss << "}";
  // ss << "]";
  // cout << ss.str() << endl;
}

int main(int argc, char** argv) {
  util::Init();
  // input
  vector<string> problem_files;
  int time_limit_seconds = 300;
  int memory_limit;
  int cores;
  vector<string> phrases_of_power;
  string replay_filename;
  bool manual_play = false;
  bool print_score = true;

  int result;
  while ((result = getopt(argc, argv, "f:t:m:c:p:r:is")) != -1) {
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
      case 'r':
        replay_filename = optarg;
        break;
      case 'i':
        manual_play = true;
        break;
      case 's':
        print_score = true;
        break;
      default:
        break;
    }
  }

  TimeKeeper time_keeper;
  time_keeper.Init(problem_files, (long long)time_limit_seconds * 1000000);

  // process
  for (int problem_id = 0; problem_id < (int)problem_files.size(); ++problem_id) {
    Game game;

    ifstream ifs(problem_files[problem_id].c_str());
    string problem((istreambuf_iterator<char>(ifs)), istreambuf_iterator<char>());;

    for (int source_seed_idx = 0; ; ++source_seed_idx) {
      bool ok = game.Init(problem, source_seed_idx, {});
      if (!ok)
        break;
      time_keeper.StartNewSeed(problem_id, source_seed_idx);

      if (manual_play) {
        EventLoopManual(game);
      } else {
        string commands;
        if (replay_filename == "") {
          auto ai = AI::CreateAI();
          ai->Init(time_keeper);
          commands = ai->Run(game);
        } else {
          std::ifstream ifs(replay_filename);
          if (!ifs.is_open()) {
            fprintf(stderr, "No such file: %s\n", replay_filename.c_str());
            exit(1);
          }
          getline(ifs, commands);
        }
        if (print_score) {
          OutputScores(game, commands);
        }
        EventLoopAI(game, commands);
      }

      break;
    }
    break;
  }
  return 0;
}
