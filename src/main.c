#include <SDL2/SDL.h>
#include <stdbool.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define CELL_SIZE 40
#define BOARD_SIZE 10
#define MAX_SHIPS 10

typedef enum { EMPTY, SHIP, HIT, MISS } Cell;

typedef struct {
    Cell cells[BOARD_SIZE][BOARD_SIZE];
    int ships_remaining;
    int ships_placed;
} Board;

void init_board(Board* board) {
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            board->cells[i][j] = EMPTY;
        }
    }
    board->ships_remaining = 5; // Example number of ships
    board->ships_placed = 0;
}

void render_board(SDL_Renderer* renderer, Board* board, int offset_x, int offset_y) {
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            SDL_Rect cell_rect = { offset_x + j * CELL_SIZE, offset_y + i * CELL_SIZE, CELL_SIZE, CELL_SIZE };
            switch (board->cells[i][j]) {
                case EMPTY:
                    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // Blue for water
                    break;
                case SHIP:
                    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Green for ships
                    break;
                case HIT:
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red for hits
                    break;
                case MISS:
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White for misses
                    break;
            }
            SDL_RenderFillRect(renderer, &cell_rect);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &cell_rect);
        }
    }
}

bool place_ship(Board* board, int x, int y) {
    if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE || board->cells[y][x] != EMPTY || board->ships_placed >= MAX_SHIPS) {
        return false;
    }
    board->cells[y][x] = SHIP;
    board->ships_placed++;
    return true;
}

bool take_shot(Board* board, int x, int y) {
    if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE) {
        return false;
    }
    if (board->cells[y][x] == SHIP) {
        board->cells[y][x] = HIT;
        board->ships_remaining--;
        return true;
    } else if (board->cells[y][x] == EMPTY) {
        board->cells[y][x] = MISS;
        return true;
    }
    return false;
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot initialize SDL: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Battleship Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot create window: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot create renderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    Board player_board, computer_board;
    init_board(&player_board);
    init_board(&computer_board);

    bool quit = false;
    bool player_turn = true;

    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                int x = event.button.x / CELL_SIZE;
                int y = event.button.y / CELL_SIZE;
                if (player_turn) {
                    if (place_ship(&player_board, x, y) && player_board.ships_placed == MAX_SHIPS) {
                        player_turn = false;
                    }
                } else {
                    take_shot(&computer_board, x, y);
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        render_board(renderer, &player_board, 0, 0);
        render_board(renderer, &computer_board, SCREEN_WIDTH / 2, 0);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}