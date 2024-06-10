#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

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
    board->ships_remaining = MAX_SHIPS; // Example number of ships
    board->ships_placed = 0;
}

void render_board(SDL_Renderer* renderer, Board* board, int offset_x, int offset_y, bool hide_ships) {
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            SDL_Rect cell_rect = { offset_x + j * CELL_SIZE, offset_y + i * CELL_SIZE, CELL_SIZE, CELL_SIZE };
            switch (board->cells[i][j]) {
                case EMPTY:
                    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // Blue for water
                    break;
                case SHIP:
                    if (hide_ships) {
                        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // Blue for hidden ships
                    } else {
                        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Green for ships
                    }
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

bool already_shot(Board* board, int x, int y) {
    return board->cells[y][x] == HIT || board->cells[y][x] == MISS;
}

bool can_place_ship(Board* board, int x, int y, int size, bool horizontal) {
    if (horizontal) {
        if (x + size > BOARD_SIZE) return false;
        for (int i = 0; i < size; ++i) {
            if (board->cells[y][x + i] != EMPTY) return false;
        }
    } else {
        if (y + size > BOARD_SIZE) return false;
        for (int i = 0; i < size; ++i) {
            if (board->cells[y + i][x] != EMPTY) return false;
        }
    }
    return true;
}

void place_ship_at(Board* board, int x, int y, int size, bool horizontal, FILE* file) {
    if (horizontal) {
        for (int i = 0; i < size; ++i) {
            board->cells[y][x + i] = SHIP;
            fprintf(file, "%d %d\n", y, x + i);
        }
    } else {
        for (int i = 0; i < size; ++i) {
            board->cells[y + i][x] = SHIP;
            fprintf(file, "%d %d\n", y + i, x);
        }
    }
}

void random_place_ship(Board* board, int ship_size, FILE* file) {
    bool placed = false;
    while (!placed) {
        int x = rand() % BOARD_SIZE;
        int y = rand() % BOARD_SIZE;
        bool horizontal = rand() % 2;

        if (can_place_ship(board, x, y, ship_size, horizontal)) {
            place_ship_at(board, x, y, ship_size, horizontal, file);
            placed = true;
        }
    }
}

void place_computer_ships(Board* board) {
    FILE* file = fopen("computer_ships.txt", "w");
    if (file == NULL) {
        printf("Error opening file for writing.\n");
        exit(1);
    }

    random_place_ship(board, 4, file); // One ship of size 4
    random_place_ship(board, 3, file); // Two ships of size 3
    random_place_ship(board, 3, file);
    random_place_ship(board, 2, file); // Four ships of size 2
    random_place_ship(board, 2, file);
    random_place_ship(board, 2, file);
    random_place_ship(board, 2, file);
    board->ships_placed = MAX_SHIPS; // Assuming MAX_SHIPS = 10 for simplicity

    fclose(file);
}

int main() {
    srand(time(NULL)); // Initialize random seed

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

    place_computer_ships(&computer_board);

    bool quit = false;
    bool player_turn = true;
    bool game_over = false;
    char winner[50] = "";

    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                int x = event.button.x / CELL_SIZE;
                int y = event.button.y / CELL_SIZE;
                if (player_turn) {
                    if (player_board.ships_placed < MAX_SHIPS) {
                        if (place_ship(&player_board, x, y) && player_board.ships_placed == MAX_SHIPS) {
                            player_turn = false; // Switch to computer's turn after placing all ships
                        }
                    } else {
                        if (take_shot(&computer_board, x - (SCREEN_WIDTH / 2) / CELL_SIZE, y)) {
                            player_turn = false; // Switch to computer's turn
                        }
                    }
                }
            }
        }

        if (!player_turn && player_board.ships_placed == MAX_SHIPS && !game_over) {
            // Computer's turn to take a shot
            int x, y;
            do {
                x = rand() % BOARD_SIZE;
                y = rand() % BOARD_SIZE;
            } while (already_shot(&player_board, x, y));
            if (take_shot(&player_board, x, y)) {
                player_turn = true; // Switch back to player's turn
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        render_board(renderer, &player_board, 0, 0, false);
        render_board(renderer, &computer_board, SCREEN_WIDTH / 2, 0, true);

        SDL_RenderPresent(renderer);

        // Check for game over
        if (player_board.ships_remaining == 0 || computer_board.ships_remaining == 0) {
            game_over = true;
            if (player_board.ships_remaining == 0) {
                snprintf(winner, sizeof(winner), "Computer wins!");
            } else {
                snprintf(winner, sizeof(winner), "Player wins!");
            }
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Game Over", winner, window);
            quit = true;
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}