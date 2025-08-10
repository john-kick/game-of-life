#include <SDL2/SDL.h>
#include <iostream>
#include <vector>

// Window
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define FRAMES_PER_SECOND 144

#define RAND_SQUARE_RADIUS 10

// Grid
#define CELL_SIZE 4
#define GRID_WIDTH (WINDOW_WIDTH / CELL_SIZE)
#define GRID_HEIGHT (WINDOW_HEIGHT / CELL_SIZE)
#define COLOR_DEAD 0x0d101c
#define COLOR_ALIVE 0xadadad
#define MOUSE_HOVER_COLOR_DEAD 0x00ffff
#define MOUSE_HOVER_COLOR_ALIVE 0xff0000

using Grid = std::vector<bool>;

struct GameState {
  bool running;
  bool paused;
  bool step;
  Grid grid;
  Grid nextGrid;
  Grid previousGrid;
};

inline bool getCell(const Grid &grid, int x, int y) {
  return grid[y * GRID_WIDTH + x];
}

inline void setCell(Grid &grid, int x, int y, bool value) {
  grid[y * GRID_WIDTH + x] = value;
}

bool getAlive(bool isAlive, uint8_t neighbors) {
  return isAlive ? (neighbors == 2 || neighbors == 3) : (neighbors == 3);
}

void updateGrid(const Grid &current, Grid &next) {
  for (int x = 0; x < GRID_WIDTH; ++x) {
    for (int y = 0; y < GRID_HEIGHT; ++y) {
      uint8_t livingNeighbors = 0;

      for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
          if (dx == 0 && dy == 0)
            continue; // Skip the cell itself
          int nx = x + dx, ny = y + dy;
          if (nx >= 0 && nx < GRID_WIDTH && ny >= 0 && ny < GRID_HEIGHT)
            livingNeighbors += getCell(current, nx, ny);
        }
      }

      bool isAlive = getCell(current, x, y);
      setCell(next, x, y, getAlive(isAlive, livingNeighbors));
    }
  }
}

void handleLeftClick(SDL_Event &event, GameState &gameState) {
  int x, y;
  SDL_GetMouseState(&x, &y);

  int gridX = x / CELL_SIZE;
  int gridY = y / CELL_SIZE;
  if (gridX >= 0 && gridX < GRID_WIDTH && gridY >= 0 && gridY < GRID_HEIGHT) {
    bool current = getCell(gameState.grid, gridX, gridY);
    setCell(gameState.grid, gridX, gridY, !current);
  }
}

void handleRightClick(SDL_Event &event, GameState &gameState) {
  int x, y;
  SDL_GetMouseState(&x, &y);

  int gridX = x / CELL_SIZE;
  int gridY = y / CELL_SIZE;

  if (gridX >= 0 && gridX < GRID_WIDTH && gridY >= 0 && gridY < GRID_HEIGHT) {
    for (int dx = -RAND_SQUARE_RADIUS; dx <= RAND_SQUARE_RADIUS; ++dx) {
      for (int dy = -RAND_SQUARE_RADIUS; dy <= RAND_SQUARE_RADIUS; ++dy) {
        int nx = gridX + dx;
        int ny = gridY + dy;
        if (nx >= 0 && nx < GRID_WIDTH && ny >= 0 && ny < GRID_HEIGHT) {
          setCell(gameState.grid, nx, ny,
                  getCell(gameState.grid, nx, ny) || rand() % 2 == 0);
        }
      }
    }
  }
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
      if (event.button.button == SDL_BUTTON_LEFT) {
        handleLeftClick(event, gameState);
      } else if (event.button.button == SDL_BUTTON_RIGHT) {
        handleRightClick(event, gameState);
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
  Grid grid(GRID_WIDTH * GRID_HEIGHT, false);

  // Block
  setCell(grid, 1, 1, true);
  setCell(grid, 1, 2, true);
  setCell(grid, 2, 1, true);
  setCell(grid, 2, 2, true);

  // Blinker
  setCell(grid, 6, 1, true);
  setCell(grid, 6, 2, true);
  setCell(grid, 6, 3, true);

  // Glider
  setCell(grid, 12, 1, true);
  setCell(grid, 13, 2, true);
  setCell(grid, 11, 3, true);
  setCell(grid, 12, 3, true);
  setCell(grid, 13, 3, true);

  return grid;
}

void drawChangedCells(SDL_Renderer *renderer, const Grid &grid,
                      const Grid &prev) {
  for (int x = 0; x < GRID_WIDTH; ++x) {
    for (int y = 0; y < GRID_HEIGHT; ++y) {
      bool current = getCell(grid, x, y);
      bool previous = getCell(prev, x, y);
      if (current != previous) {
        SDL_SetRenderDrawColor(
            renderer, current ? (COLOR_ALIVE >> 16) : (COLOR_DEAD >> 16),
            current ? ((COLOR_ALIVE >> 8) & 0xFF) : ((COLOR_DEAD >> 8) & 0xFF),
            current ? (COLOR_ALIVE & 0xFF) : (COLOR_DEAD & 0xFF), 255);
        SDL_Rect cell = {x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE};
        SDL_RenderFillRect(renderer, &cell);
      }
    }
  }
}

void drawMouseHover(const Grid &grid, SDL_Renderer *renderer, int &prevHoverX,
                    int &prevHoverY) {
  int mouseX, mouseY;
  SDL_GetMouseState(&mouseX, &mouseY);
  int hoverX = mouseX / CELL_SIZE;
  int hoverY = mouseY / CELL_SIZE;

  // Clear previous hover by redrawing cell state
  if (prevHoverX != -1 && prevHoverY != -1 &&
      (prevHoverX != hoverX || prevHoverY != hoverY)) {
    bool prevAlive = getCell(grid, prevHoverX, prevHoverY);
    SDL_SetRenderDrawColor(
        renderer, prevAlive ? (COLOR_ALIVE >> 16) : (COLOR_DEAD >> 16),
        prevAlive ? ((COLOR_ALIVE >> 8) & 0xFF) : ((COLOR_DEAD >> 8) & 0xFF),
        prevAlive ? (COLOR_ALIVE & 0xFF) : (COLOR_DEAD & 0xFF), 255);
    SDL_Rect prevRect = {prevHoverX * CELL_SIZE, prevHoverY * CELL_SIZE,
                         CELL_SIZE, CELL_SIZE};
    SDL_RenderFillRect(renderer, &prevRect);
  }

  // Draw new hover
  if (hoverX >= 0 && hoverX < GRID_WIDTH && hoverY >= 0 &&
      hoverY < GRID_HEIGHT) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(
        renderer,
        getCell(grid, hoverX, hoverY) ? (MOUSE_HOVER_COLOR_ALIVE >> 16)
                                      : (MOUSE_HOVER_COLOR_DEAD >> 16),
        getCell(grid, hoverX, hoverY) ? ((MOUSE_HOVER_COLOR_ALIVE >> 8) & 0xFF)
                                      : ((MOUSE_HOVER_COLOR_DEAD >> 8) & 0xFF),
        getCell(grid, hoverX, hoverY) ? (MOUSE_HOVER_COLOR_ALIVE & 0xFF)
                                      : (MOUSE_HOVER_COLOR_DEAD & 0xFF),
        255);
    SDL_Rect hoverRect = {hoverX * CELL_SIZE, hoverY * CELL_SIZE, CELL_SIZE,
                          CELL_SIZE};
    SDL_RenderFillRect(renderer, &hoverRect);

    // Update previous hover
    prevHoverX = hoverX;
    prevHoverY = hoverY;
  }
}

void draw(SDL_Renderer *renderer, const Grid &grid, const Grid &previousGrid,
          const GameState gameState, int &prevHoverX, int &prevHoverY) {
  drawChangedCells(renderer, grid, previousGrid);
  drawMouseHover(grid, renderer, prevHoverX, prevHoverY);
  SDL_RenderPresent(renderer);
}

int main() {
  SDL_Window *window = nullptr;
  SDL_Renderer *renderer = nullptr;

  initSDL();
  createWindowAndRenderer(window, renderer);

  GameState gameState = {
      .running = true,
      .paused = true,
      .step = false,
      .grid = initializeGrid(),
      .nextGrid = Grid(GRID_WIDTH * GRID_HEIGHT, false),
      .previousGrid = Grid(GRID_WIDTH * GRID_HEIGHT, false),
  };
  int prevHoverX = -1, prevHoverY = -1;

  // Initial draw
  SDL_SetRenderDrawColor(renderer, (COLOR_DEAD >> 16),
                         ((COLOR_DEAD >> 8) & 0xFF), (COLOR_DEAD & 0xFF), 255);
  SDL_Rect gameField = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
  SDL_RenderFillRect(renderer, &gameField);

  SDL_Event event;
  while (gameState.running) {
    handleEvents(event, gameState);

    draw(renderer, gameState.grid, gameState.previousGrid, gameState,
         prevHoverX, prevHoverY);

    if (!gameState.paused || gameState.step) {
      gameState.step = false;
      updateGrid(gameState.grid, gameState.nextGrid);
      gameState.previousGrid = gameState.grid;
      std::swap(gameState.grid, gameState.nextGrid);
    }

    // SDL_Delay(1000 / FRAMES_PER_SECOND);
  }

  cleanupSDL(window, renderer);
  return 0;
}
