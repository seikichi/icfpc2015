#include "key_input.h"

#include <assert.h>
#include <SDL.h>

void KeyInput::Init() {
  memset(prev_keys, 0, sizeof(prev_keys));
  memset(now_keys, 0, sizeof(now_keys));
  target_keys = {
    { 'a', SDLK_a },
    { 'z', SDLK_z },
    { 'x', SDLK_x },
    { 'd', SDLK_d },
    { 'j', SDLK_j },
    { 'k', SDLK_k },
    { ' ', SDLK_SPACE },
  };
}
void KeyInput::Update() {
  memcpy(prev_keys, now_keys, sizeof(prev_keys));
  const Uint8* keys = SDL_GetKeyboardState(NULL);
  for (auto key : target_keys) {
    now_keys[(int)key.first] = keys[SDL_GetScancodeFromKey(key.second)];
  }
}
bool KeyInput::Pushed(int c) {
  assert(target_keys.count(c));
  return now_keys[c] && !prev_keys[c];
}
