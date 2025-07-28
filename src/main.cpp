#include <SDL2/SDL.h>
#include <iostream>
#include <vector>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define CELL_SIZE 10
#define GRID_WIDTH (WINDOW_WIDTH / CELL_SIZE)
#define GRID_HEIGHT (WINDOW_HEIGHT / CELL_SIZE)
#define FRAMES_PER_SECOND 144
#define COLOR_DEAD 0x0d101c
#define COLOR_ALIVE 0xadadad

using Grid = std::vector<std::vector<bool>>;

struct GameState {
  bool running;
  bool paused;
  bool step;
  Grid grid;
};

bool getAlive(bool isAlive, uint8_t neighbors) {
  return isAlive ? (neighbors == 2 || neighbors == 3) : (neighbors == 3);
}

Grid updateGrid(const Grid &oldGrid) {
  Grid newGrid(GRID_WIDTH, std::vector<bool>(GRID_HEIGHT, false));

  for (int x = 0; x < GRID_WIDTH; ++x) {
    for (int y = 0; y < GRID_HEIGHT; ++y) {
      uint8_t livingNeighbors = 0;

      for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
          if (dx == 0 && dy == 0)
            continue;
          int nx = x + dx, ny = y + dy;
          if (nx >= 0 && nx < GRID_WIDTH && ny >= 0 && ny < GRID_HEIGHT)
            livingNeighbors += oldGrid[nx][ny];
        }
      }

      newGrid[x][y] = getAlive(oldGrid[x][y], livingNeighbors);
    }
  }

  return newGrid;
}

void handleEvents(SDL_Event &event, GameState &gameState) {
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT)
      gameState.running = false;
    else if (event.type == SDL_KEYDOWN) {
      if (event.key.keysym.sym == SDLK_SPACE) {
        gameState.paused = !gameState.paused;
      } else if (event.key.keysym.sym == SDLK_RIGHT) {
        gameState.step = true;
      }
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
      int x, y;
      SDL_GetMouseState(&x, &y);
      int gridX = x / CELL_SIZE;
      int gridY = y / CELL_SIZE;
      if (gridX >= 0 && gridX < GRID_WIDTH && gridY >= 0 &&
          gridY < GRID_HEIGHT) {
        gameState.grid[gridX][gridY] = !gameState.grid[gridX][gridY];
      }
    }
  }
}

void initSDL() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError()
              << '\n';
    exit(1);
  }
}

void cleanupSDL(SDL_Window *window, SDL_Renderer *renderer) {
  if (renderer)
    SDL_DestroyRenderer(renderer);
  if (window)
    SDL_DestroyWindow(window);
  SDL_Quit();
}

void createWindowAndRenderer(SDL_Window *&window, SDL_Renderer *&renderer) {
  window =
      SDL_CreateWindow("Game of Life", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
  if (!window) {
    std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError()
              << '\n';
    cleanupSDL(window, renderer);
    exit(1);
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
  if (!renderer) {
    std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError()
              << '\n';
    cleanupSDL(window, renderer);
    exit(1);
  }
}

Grid initializeGrid() {
  Grid grid(GRID_WIDTH, std::vector<bool>(GRID_HEIGHT, false));

  // Block
  grid[1][1] = grid[2][1] = true;
  grid[1][2] = grid[2][2] = true;

  // Blinker
  grid[6][1] = grid[6][2] = grid[6][3] = true;

  // Glider
  grid[12][1] = grid[13][2] = true;
  grid[11][3] = grid[12][3] = grid[13][3] = true;

  return grid;
}

void clearGrid(SDL_Renderer *renderer) {
  SDL_SetRenderDrawColor(renderer, COLOR_DEAD >> 16, (COLOR_DEAD >> 8) & 0xFF,
                         COLOR_DEAD & 0xFF, 255);
  SDL_RenderClear(renderer);
}

void draw(SDL_Renderer *renderer, const Grid &grid) {
  for (int x = 0; x < GRID_WIDTH; ++x) {
    for (int y = 0; y < GRID_HEIGHT; ++y) {
      if (grid[x][y]) {
        SDL_SetRenderDrawColor(renderer, COLOR_ALIVE >> 16,
                               (COLOR_ALIVE >> 8) & 0xFF, COLOR_ALIVE & 0xFF,
                               255);
        SDL_Rect cell = {x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE};
        SDL_RenderFillRect(renderer, &cell);
      }
    }
  }

  int mouseX, mouseY;
  SDL_GetMouseState(&mouseX, &mouseY);
  int hoverX = mouseX / CELL_SIZE;
  int hoverY = mouseY / CELL_SIZE;

  if (hoverX >= 0 && hoverX < GRID_WIDTH && hoverY >= 0 &&
      hoverY < GRID_HEIGHT) {
    if (grid[hoverX][hoverY]) {
      SDL_SetRenderDrawColor(renderer, 255, 0, 0, 128);
    } else {
      SDL_SetRenderDrawColor(renderer, 0, 255, 255, 128);
    }
    SDL_Rect hoverRect = {hoverX * CELL_SIZE, hoverY * CELL_SIZE, CELL_SIZE,
                          CELL_SIZE};
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderFillRect(renderer, &hoverRect);
  }

  SDL_RenderPresent(renderer);
}

int main() {
  SDL_Window *window = nullptr;
  SDL_Renderer *renderer = nullptr;

  initSDL();
  createWindowAndRenderer(window, renderer);

  GameState gameState = {true, true, false, initializeGrid()};

  SDL_Event event;
  while (gameState.running) {
    clearGrid(renderer);
    handleEvents(event, gameState);
    draw(renderer, gameState.grid);

    if (!gameState.paused || gameState.step) {
      gameState.step = false;
      gameState.grid = updateGrid(gameState.grid);
    }

    SDL_Delay(1000 / FRAMES_PER_SECOND);
  }

  cleanupSDL(window, renderer);
  return 0;
}
