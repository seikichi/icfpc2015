#pragma once

#include <map>

struct KeyInput {
  int prev_keys[256];
  int now_keys[256];
  std::map<char, int> target_keys;
  void Init();
  void Update();
  bool Pushed(int c);
};
