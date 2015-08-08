#include "visualizer.h"
#include "ai.h"
#include "game.h"

#include <cstdio>
#include <unistd.h>

#include <fstream>
#include <memory>
#include <sstream>
#include <streambuf>
#include <string>
#include <vector>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include "picojson/picojson.h"

using namespace std;

bool Visualizer::CreateSDL(const Game &game) {
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0){
    std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
    exit(1);
  }

  int width = GetBoardWidth(game) + 64;
  int height = GetBoardHeight(game) + 128;
  win = SDL_CreateWindow("Honeycomb Tetris", 100, 100, width, height, SDL_WINDOW_SHOWN);

  if (win == nullptr){
    std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
    SDL_Quit();
    return false;
    // exit(1);
  }
  ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (ren == nullptr){
    SDL_DestroyWindow(win);
    std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
    SDL_Quit();
    return false;
    // exit(1);
  }

  std::string imagePath = "resources/tiles.png";
  tex = LoadTexture(imagePath, ren);
  if (tex == nullptr){
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    std::cout << "SDL_CreateTextureFromSurface Error: " << SDL_GetError() << std::endl;
    SDL_Quit();
    return false;
    // exit(1);
  }
  std::string letterPath = "resources/letters.png";
  letters_image = LoadTexture(letterPath, ren);
  if (tex == nullptr){
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    std::cout << "SDL_CreateTextureFromSurface Error: " << SDL_GetError() << std::endl;
    SDL_Quit();
    return false;
    // exit(1);
  }
  SDL_RenderClear(ren);
  SDL_RenderPresent(ren);
  return true;
}

SDL_Texture* Visualizer::LoadTexture(const std::string &file, SDL_Renderer *ren){
  SDL_Texture *texture = IMG_LoadTexture(ren, file.c_str());
  if (texture == nullptr)     
    std::cout << "LoadTexture Error:" << SDL_GetError() << std::endl;
  return texture;
}

void Visualizer::DestroySDL() {
  SDL_DestroyTexture(tex);
  SDL_DestroyRenderer(ren);
  SDL_DestroyWindow(win);
  SDL_Quit();
}

/* (x, y)にテキスト(ただし、数字、アルファベット、空白のみ)を表示する。
 * printf と同じような書き方ができる。
 */
void Visualizer::DrawText(int x, int y, const char* format, ...)
{
  int i;
  va_list ap;
  char buffer[256];

  va_start(ap, format);
  vsnprintf(buffer, 255, format, ap);
  va_end(ap);
  buffer[255] = '\0';
  for (i = 0; buffer[i] != '\0'; ++i) {
    SDL_Rect srcrect;
    SDL_Rect destrect = { x + i * 10, y, 10, 20 };
    if (isdigit(buffer[i])) {           /* 数字 */
      srcrect.x = (buffer[i] - '0') * 10;
      srcrect.y = 20;
    } else if (isalpha(buffer[i])) {    /* アルファベット */
      srcrect.x = (toupper(buffer[i]) - 'A') * 10;
      srcrect.y = 0;
    } else {                            /* それ以外は空白とみなす */
      continue;
    }
    srcrect.w = 10;
    srcrect.h = 20;
    destrect.w = 10;
    destrect.h = 20;
    SDL_RenderCopy(ren, letters_image, &srcrect, &destrect);
  }
}

void Visualizer::Draw(int x, int y, Image image) {
  SDL_Rect srcrect, destrect;
  srcrect.x = (int)image * TILE_SIZE;
  srcrect.y = 0;
  srcrect.w = TILE_SIZE;
  srcrect.h = TILE_SIZE;
  destrect.x = x;
  destrect.y = y;
  destrect.w = TILE_SIZE;
  destrect.h = TILE_SIZE;
  SDL_RenderCopy(ren, tex, &srcrect, &destrect);
}
int Visualizer::CellX(const Cell &cell) {
  const int dx = TILE_SIZE;
  const int offset = TILE_SIZE / 2;
  return dx * cell.x + (cell.y % 2 * offset);
}
int Visualizer::CellY(const Cell & cell) {
  const int dy = TILE_SIZE - 4;
  return cell.y * dy;
}

void Visualizer::BeginDraw() {
  SDL_RenderClear(ren);
}
void Visualizer::DrawGameState(const Game &game, const State &state) {
  for (int y = 0; y < game.h; y++) {
    for (int x = 0; x < game.w; x++) {
      Cell cell(x, y);
      if (state.board[cell.Lin(game.w)]) {
       Draw(CellX(cell), CellY(cell), Image::FILLED);
      } else {
        Draw(CellX(cell), CellY(cell), Image::EMPTY);
      }
    }
  }
  for (const Cell& cell : state.GetCurrentUnitCells(game)) {
    Draw(CellX(cell), CellY(cell), Image::UNIT);
  }
  Draw(CellX(state.pivot), CellY(state.pivot), Image::PIVOT);
  DrawText(8, GetBoardHeight(game) + 8, "Score: %d", state.score);
  if (state.gameover) {
    DrawText(8, GetBoardHeight(game) + 8 + 24, "Game Over!");
  }
}
void Visualizer::DrawCommandResult(const Game& game, const CommandResult result) {
  map<CommandResult, string> message {
    { MOVE, "" },
    { LOCK, "Lock" },
    { ERROR, "Error" },
    { CLEAR, "Clear" },
    { GAMEOVER, "Game End" },
  };
  DrawText(8, GetBoardHeight(game) + 8 + 48, "%s", message[result].c_str());
}
void Visualizer::EndDraw() {
  SDL_RenderPresent(ren);
}
