#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
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

typedef enum { MENU, PLAYING } GameState;

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

void render_text(SDL_Renderer* renderer, TTF_Font* font, const char* text, int x, int y) {
    SDL_Color color = { 255, 255, 255, 255 };
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    int text_width = surface->w;
    int text_height = surface->h;
    SDL_Rect render_quad = { x, y, text_width, text_height };

    SDL_RenderCopy(renderer, texture, NULL, &render_quad);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
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

void animate_hit_miss(SDL_Renderer* renderer, int x, int y, bool is_hit) {
    SDL_Rect rect = { x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE };
    SDL_Color color = is_hit ? (SDL_Color){255, 0, 0, 255} : (SDL_Color){255, 255, 255, 255}; // Red for hit, White for miss

    for (int i = 0; i < 5; ++i) {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(renderer, &rect);
        SDL_RenderPresent(renderer);
        SDL_Delay(100); // Delay for animation effect

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(renderer, &rect);
        SDL_RenderPresent(renderer);
        SDL_Delay(100);
    }
}

void save_game(Board* player_board, Board* computer_board) {
    FILE* file = fopen("savegame.dat", "wb");
    if (file == NULL) {
        printf("Error opening file for saving.\n");
        return;
    }
    fwrite(player_board, sizeof(Board), 1, file);
    fwrite(computer_board, sizeof(Board), 1, file);
    fclose(file);
}

bool load_game(Board* player_board, Board* computer_board) {
    FILE* file = fopen("savegame.dat", "rb");
    if (file == NULL) {
        printf("Error opening file for loading.\n");
        return false;
    }
    fread(player_board, sizeof(Board), 1, file);
    fread(computer_board, sizeof(Board), 1, file);
    fclose(file);
    return true;
}

void render_menu(SDL_Renderer* renderer, TTF_Font* font) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    render_text(renderer, font, "Battleship Game", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 100);
    render_text(renderer, font, "1. Start New Game", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 50);
    render_text(renderer, font, "2. Load Game", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2);
    SDL_RenderPresent(renderer);
}

int main() {
    srand(time(NULL)); // Initialize random seed

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot initialize SDL: %s", SDL_GetError());
        return 1;
    }

    if (TTF_Init() < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot initialize SDL_ttf: %s", TTF_GetError());
        SDL_Quit();
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

    TTF_Font* font = TTF_OpenFont("path/to/font.ttf", 24);
    if (font == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot open font: %s", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    GameState game_state = MENU;
    Board player_board, computer_board;

    bool quit = false;
    bool player_turn = true;
    bool game_over = false;
    char winner[50] = "";

    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            } else if (event.type == SDL_KEYDOWN) {
                if (game_state == MENU) {
                    if (event.key.keysym.sym == SDLK_1) {
                        game_state = PLAYING;
                        init_board(&player_board);
                        init_board(&computer_board);
                        place_computer_ships(&computer_board);
                    } else if (event.key.keysym.sym == SDLK_2) {
                        if (load_game(&player_board, &computer_board)) {
                            game_state = PLAYING;
                        }
                    }
                }
            } else if (event.type == SDL_MOUSEBUTTONDOWN && game_state == PLAYING) {
                int x = event.button.x / CELL_SIZE;
                int y = event.button.y / CELL_SIZE;
                if (player_turn) {
                    if (player_board.ships_placed < MAX_SHIPS) {
                        if (place_ship(&player_board, x, y) && player_board.ships_placed == MAX_SHIPS) {
                            player_turn = false; // Switch to computer's turn after placing all ships
                        }
                    } else {
                        bool is_hit = take_shot(&computer_board, x - (SCREEN_WIDTH / 2) / CELL_SIZE, y);
                        animate_hit_miss(renderer, x, y, is_hit);
                        player_turn = false; // Switch to computer's turn
                    }
                }
            }
        }

        if (game_state == MENU) {
            render_menu(renderer, font);
        } else if (game_state == PLAYING) {
            if (!player_turn && player_board.ships_placed == MAX_SHIPS && !game_over) {
                // Computer's turn to take a shot
                int x, y;
                do {
                    x = rand() % BOARD_SIZE;
                    y = rand() % BOARD_SIZE;
                } while (already_shot(&player_board, x, y));
                bool is_hit = take_shot(&player_board, x, y);
                animate_hit_miss(renderer, x + (SCREEN_WIDTH / 2) / CELL_SIZE, y, is_hit);
                player_turn = true; // Switch back to player's turn
            }

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            render_board(renderer, &player_board, 0, 0, false);
            render_board(renderer, &computer_board, SCREEN_WIDTH / 2, 0, true);

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawLine(renderer, SCREEN_WIDTH / 2, 0, SCREEN_WIDTH / 2, SCREEN_HEIGHT);

            char player_ships[50];
            char computer_ships[50];
            snprintf(player_ships, sizeof(player_ships), "Player Ships Remaining: %d", player_board.ships_remaining);
            snprintf(computer_ships, sizeof(computer_ships), "Computer Ships Remaining: %d", computer_board.ships_remaining);

            render_text(renderer, font, player_ships, 10, SCREEN_HEIGHT - 30);
            render_text(renderer, font, computer_ships, SCREEN_WIDTH / 2 + 10, SCREEN_HEIGHT - 30);

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
                save_game(&player_board, &computer_board);
                quit = true;
            }
        }
    }

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}