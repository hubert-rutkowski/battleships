#include <SDL2/SDL.h>
#include <stdbool.h>  // Dodaj to

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

int main() {
    // Inicjalizacja SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Nie można zainicjalizować SDL: %s", SDL_GetError());
        return 1;
    }

    // Tworzenie okna
    SDL_Window* window = SDL_CreateWindow("Gra w statki", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Nie można utworzyć okna: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Tworzenie rendera
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Nie można utworzyć renderera: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Główna pętla gry
    bool quit = false;

    while (!quit) {
        // Obsługa zdarzeń
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }

        // Czyszczenie ekranu
        SDL_RenderClear(renderer);

        // Aktualizacja ekranu
        SDL_RenderPresent(renderer);
    }

    // Zwalnianie zasobów
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}