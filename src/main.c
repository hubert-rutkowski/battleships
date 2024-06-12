#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define CELL_SIZE 30
#define BOARD_SIZE 10
#define PADDING 50

typedef enum { EMPTY, SHIP, MISS, HIT } Cell;
typedef enum { MENU, PLACING_SHIPS, PLAYING, GAME_OVER } GameState;

typedef struct {
    Cell cells[BOARD_SIZE][BOARD_SIZE];
    int ships_placed;
    int ships_remaining;
} Board;

void init_board(Board* board) {
    for (int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            board->cells[y][x] = EMPTY;
        }
    }
    board->ships_placed = 0;
}

bool place_ship(Board* board, int x, int y) {
    if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE || board->cells[y][x] != EMPTY) {
        return false;
    }

    board->cells[y][x] = SHIP;
    board->ships_placed++;
    board->ships_remaining++;
    return true;
}

int take_shot(Board* board, int x, int y) {
    if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE || board->cells[y][x] == MISS || board->cells[y][x] == HIT) {
        return -1;
    }

    if (board->cells[y][x] == SHIP) {
        board->cells[y][x] = HIT;
        board->ships_remaining--;
        return true;
    } else {
        board->cells[y][x] = MISS;
        return false;
    }
}

void computer_take_shot(Board* player_board) {
    int board_x = rand() % BOARD_SIZE;
    int board_y = rand() % BOARD_SIZE;

    while (player_board->cells[board_y][board_x] != EMPTY) {
        board_x = rand() % BOARD_SIZE;
        board_y = rand() % BOARD_SIZE;
    }

    take_shot(player_board, board_x, board_y);
}

void render_board(SDL_Renderer* renderer, Board* board, int offset_x, int offset_y, bool hide_ships) {
    for (int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            SDL_Rect cell_rect = { offset_x + x * CELL_SIZE, offset_y + y * CELL_SIZE, CELL_SIZE, CELL_SIZE };
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
            SDL_RenderFillRect(renderer, &cell_rect);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &cell_rect);

            if (board->cells[y][x] == SHIP && !hide_ships) {
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                SDL_RenderFillRect(renderer, &cell_rect);
            } else if (board->cells[y][x] == MISS) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderFillRect(renderer, &cell_rect);
            } else if (board->cells[y][x] == HIT) {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                SDL_RenderFillRect(renderer, &cell_rect);
            }
        }
    }
}

void render_text(SDL_Renderer* renderer, TTF_Font* font, const char* text, int x, int y) {
    SDL_Color color = { 255, 255, 255, 255 };
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dstrect = { x, y, surface->w, surface->h };
    SDL_RenderCopy(renderer, texture, NULL, &dstrect);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void render_menu(SDL_Renderer* renderer, TTF_Font* font) {
    render_text(renderer, font, "New Game", SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 2 - 30);
    render_text(renderer, font, "Load Game", SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 2);
    render_text(renderer, font, "Quit", SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 2 + 30);
}

int handle_menu_click(int mouse_x, int mouse_y) {
    if (mouse_x >= SCREEN_WIDTH / 2 - 50 && mouse_x <= SCREEN_WIDTH / 2 + 50) {
        if (mouse_y >= SCREEN_HEIGHT / 2 - 30 && mouse_y <= SCREEN_HEIGHT / 2) {
            return 1; // New Game
        } else if (mouse_y >= SCREEN_HEIGHT / 2 && mouse_y <= SCREEN_HEIGHT / 2 + 30) {
            return 2; // Load Game
        } else if (mouse_y >= SCREEN_HEIGHT / 2 + 30 && mouse_y <= SCREEN_HEIGHT / 2 + 60) {
            return 3; // Quit
        }
    }
    return 0;
}

void render_save_button(SDL_Renderer* renderer, TTF_Font* font) {
    render_text(renderer, font, "Save Game", SCREEN_WIDTH - 130, 10);
}

bool handle_save_button(int mouse_x, int mouse_y) {
    return mouse_x >= SCREEN_WIDTH - 120 && mouse_x <= SCREEN_WIDTH - 20 && mouse_y >= 10 && mouse_y <= 40;
}

void place_computer_ships(Board* board) {
    int ship_lengths[] = {4, 3, 3}; 
    int num_ships = sizeof(ship_lengths) / sizeof(ship_lengths[0]);
    int sum_lengths = 0;
    for (int i = 0; i < num_ships; i++) {
        sum_lengths += ship_lengths[i];
    }
    board->ships_remaining = sum_lengths;
    for (int i = 0; i < num_ships; i++) {
        bool ship_placed = false;
        while (!ship_placed) {
            int x = rand() % BOARD_SIZE;
            int y = rand() % BOARD_SIZE;
            int direction = rand() % 2;

            bool can_place = true;
            for (int j = 0; j < ship_lengths[i]; j++) {
                int ship_x = x + (direction == 0 ? j : 0);
                int ship_y = y + (direction == 1 ? j : 0);
                if (ship_x < 0 || ship_y < 0 || ship_x >= BOARD_SIZE || ship_y >= BOARD_SIZE || board->cells[ship_y][ship_x] != EMPTY) {
                    can_place = false;
                    break;
                }
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        int nx = ship_x + dx;
                        int ny = ship_y + dy;
                        if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE && board->cells[ny][nx] == SHIP) {
                            can_place = false;
                            break;
                        }
                    }
                    if (!can_place) break;
                }
            }

            if (can_place) {
                for (int j = 0; j < ship_lengths[i]; j++) {
                    int ship_x = x + (direction == 0 ? j : 0);
                    int ship_y = y + (direction == 1 ? j : 0);
                    board->cells[ship_y][ship_x] = SHIP;
                }
                ship_placed = true;
            }
        }
    }
}

bool load_game(Board* player_board, Board* computer_board) {
    FILE* file = fopen("battleships_save.dat", "rb");
    if (file == NULL) {
        return false;
    }

    fread(player_board, sizeof(Board), 1, file);
    fread(computer_board, sizeof(Board), 1, file);
    fclose(file);

    return true;
}

void save_game(Board* player_board, Board* computer_board) {
    FILE* file = fopen("battleships_save.dat", "wb");
    if (file == NULL) {
        printf("Failed to save game.\n");
        return;
    }

    fwrite(player_board, sizeof(Board), 1, file);
    fwrite(computer_board, sizeof(Board), 1, file);
    fclose(file);

    printf("Game saved successfully.\n");
}

bool already_shot(Board* board, int x, int y) {
    return board->cells[y][x] == MISS || board->cells[y][x] == HIT;
}

void animate_hit_miss(SDL_Renderer* renderer, int x, int y, bool is_hit, int offset_x, int offset_y) {
    SDL_Rect rect = { offset_x + x * CELL_SIZE, offset_y + y * CELL_SIZE, CELL_SIZE, CELL_SIZE };
    SDL_Color color = is_hit ? (SDL_Color){255, 0, 0, 255} : (SDL_Color){255, 255, 255, 255};

    for (int i = 0; i < 5; ++i) {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(renderer, &rect);
        SDL_RenderPresent(renderer);
        SDL_Delay(100);
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        SDL_RenderFillRect(renderer, &rect);
        SDL_RenderPresent(renderer);
        SDL_Delay(100);
    }
}

void render_game_over_menu(SDL_Renderer* renderer, TTF_Font* font, const char* winner) {
    render_text(renderer, font, winner, SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 2 - 60);
    render_text(renderer, font, "Play again", SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 2);
    render_text(renderer, font, "Exit", SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 2 + 30);
}

int handle_game_over_click(int mouse_x, int mouse_y) {
    if (mouse_x >= SCREEN_WIDTH / 2 - 50 && mouse_x <= SCREEN_WIDTH / 2 + 50) {
        if (mouse_y >= SCREEN_HEIGHT / 2 && mouse_y <= SCREEN_HEIGHT / 2 + 30) {
            return 1; // Menu
        } else if (mouse_y >= SCREEN_HEIGHT / 2 + 30 && mouse_y <= SCREEN_HEIGHT / 2 + 60) {
            return 2; // Exit
        }
    }
    return 0;
}

void render_ships_remaining(SDL_Renderer* renderer, TTF_Font* font, int player_ships, int computer_ships) {
    char player_ships_text[50];
    char computer_ships_text[50];
    snprintf(player_ships_text, sizeof(player_ships_text), "Player Ships Remaining: %d", player_ships);
    snprintf(computer_ships_text, sizeof(computer_ships_text), "Computer Ships Remaining: %d", computer_ships);
    render_text(renderer, font, player_ships_text, 10, SCREEN_HEIGHT - 40);
    render_text(renderer, font, computer_ships_text, SCREEN_WIDTH / 2 + 50, SCREEN_HEIGHT - 40);
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    SDL_Window* window = SDL_CreateWindow("Battleships Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    TTF_Font* font = TTF_OpenFont("assets/OpenSans-Regular.ttf", 24);
    if (!font) {
        printf("Failed to load font: %s\n", TTF_GetError());
        return -1;
    }

    srand(time(NULL));

    GameState game_state = MENU;
    Board player_board;
    Board computer_board;
    bool player_turn = true;
    char winner[20] = "";

    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                int mouse_x, mouse_y;
                SDL_GetMouseState(&mouse_x, &mouse_y);

                if (game_state == MENU) {
                    int choice = handle_menu_click(mouse_x, mouse_y);
                    if (choice == 1) {
                        game_state = PLACING_SHIPS;
                        init_board(&player_board);
                        init_board(&computer_board);
                        place_computer_ships(&computer_board);
                    } else if (choice == 2) {
                        if (load_game(&player_board, &computer_board)) {
                            game_state = PLAYING;
                        } else {
                            printf("Failed to load game.\n");
                        }
                    } else if (choice == 3) {
                        quit = true;
                    }
                } else if (game_state == PLACING_SHIPS) {
                    int board_x = (mouse_x - PADDING) / CELL_SIZE;
                    int board_y = (mouse_y - PADDING) / CELL_SIZE;

                    if (place_ship(&player_board, board_x, board_y)) {
                        if (player_board.ships_placed == 10) {
                            game_state = PLAYING;
                        }
                    }
                } else if (game_state == PLAYING) {
                    if (handle_save_button(mouse_x, mouse_y)) {
                        save_game(&player_board, &computer_board);
                    } else {
                        int board_x = (mouse_x - (SCREEN_WIDTH - PADDING - BOARD_SIZE * CELL_SIZE)) / CELL_SIZE;
                        int board_y = (mouse_y - PADDING) / CELL_SIZE;

                        switch (take_shot(&computer_board, board_x, board_y)) {
                            case -1:
                                break;
                            case 0:
                                player_turn = false;
                                if (player_board.ships_remaining == 0) {
                                    snprintf(winner, sizeof(winner), "Computer wins!");
                                    game_state = GAME_OVER;
                                }
                                break;
                            case 1:
                                animate_hit_miss(renderer, board_x, board_y, computer_board.cells[board_y][board_x] == HIT, SCREEN_WIDTH - PADDING - BOARD_SIZE * CELL_SIZE, PADDING);
                                if (computer_board.ships_remaining == 0) {
                                    snprintf(winner, sizeof(winner), "Player wins!");
                                    game_state = GAME_OVER;
                                }
                                break;
                        }
                    }
                } else if (game_state == GAME_OVER) {
                    int choice = handle_game_over_click(mouse_x, mouse_y);
                    if (choice == 1) {
                        game_state = MENU;
                    } else if (choice == 2) {
                        quit = true;
                    }
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (game_state == MENU) {
            render_menu(renderer, font);
        } else if (game_state == PLACING_SHIPS || game_state == PLAYING) {
            render_board(renderer, &player_board, PADDING, PADDING, false);
            render_board(renderer, &computer_board, SCREEN_WIDTH - PADDING - BOARD_SIZE * CELL_SIZE, PADDING, true);
            if (game_state == PLAYING) {
                render_save_button(renderer, font);
                render_ships_remaining(renderer, font, player_board.ships_remaining, computer_board.ships_remaining);
            }
        } else if (game_state == GAME_OVER) {
            render_game_over_menu(renderer, font, winner);
        }

        SDL_RenderPresent(renderer);

        if (!player_turn && game_state == PLAYING) {
            SDL_Delay(500);
            int x, y;
            do {
                x = rand() % BOARD_SIZE;
                y = rand() % BOARD_SIZE;
            } while (already_shot(&player_board, x, y));
            bool is_hit = take_shot(&player_board, x, y);
            if (is_hit) {
                animate_hit_miss(renderer, x, y, is_hit, PADDING, PADDING);
            }
            if (player_board.ships_remaining == 0) {
                snprintf(winner, sizeof(winner), "Computer wins!");
                game_state = GAME_OVER;
            } else {
                player_turn = true;
            }
        }
    }

    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}