#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include "gameofLife.h"

// Configuración de la ventana y renderizador
const int WIDTH = 800;
const int HEIGHT = 600;

// Función para crear la ventana
SDL_Window* createWindow(const char* title, int width, int height) {
    SDL_Window* window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
    }
    return window;
}

// Función para crear el renderizador
SDL_Renderer* createRenderer(SDL_Window* window) {
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
    }
    return renderer;
}

// Función para cargar el GIF
IMG_Animation* loadGIF(const char* filePath) {
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
        return nullptr;
    }

    SDL_RWops* rwop = SDL_RWFromFile(filePath, "rb");
    if (!rwop) {
        std::cerr << "Failed to load GIF! SDL Error: " << SDL_GetError() << std::endl;
        IMG_Quit();
        return nullptr;
    }

    IMG_Animation* gifAnimation = IMG_LoadGIFAnimation_RW(rwop);
    SDL_RWclose(rwop);

    if (!gifAnimation) {
        std::cerr << "Failed to load GIF animation! SDL_image Error: " << IMG_GetError() << std::endl;
        IMG_Quit();
        return nullptr;
    }

    return gifAnimation;
}

// Función para renderizar el GIF en la pantalla
void renderGIF(SDL_Renderer* renderer, IMG_Animation* gifAnimation, int x, int y, int w, int h) {
    static int frame = 0;
    static Uint32 frameTime = SDL_GetTicks();

    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - frameTime >= gifAnimation->delays[frame]) {
        frame = (frame + 1) % gifAnimation->count;
        frameTime = currentTime;
    }

    SDL_Rect dstRect = { x, y, w, h };
    SDL_Texture* frameTexture = SDL_CreateTextureFromSurface(renderer, gifAnimation->frames[frame]);

    SDL_RenderCopy(renderer, frameTexture, NULL, &dstRect);
    SDL_DestroyTexture(frameTexture);
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = createWindow("Screen Saver", WIDTH, HEIGHT);
    if (!window) return 1;

    SDL_Renderer* renderer = createRenderer(window);
    if (!renderer) return 1;

    IMG_Animation* gifAnimation = loadGIF("files/nyancat1.gif");
    if (!gifAnimation) return 1;

    std::srand(std::time(nullptr));
    initializeGameOfLife();

    bool running = true;
    SDL_Event e;

    while (running) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                running = false;
            }
        }

        updateGameOfLife();
        
        SDL_RenderClear(renderer);
        renderBuffer(renderer); // Renderiza el Game of Life en el fondo
        renderGIF(renderer, gifAnimation, 100, 100, 250, 300); // Renderiza el GIF sobre el Game of Life
        SDL_RenderPresent(renderer);

        SDL_Delay(55);
    }

    for (int i = 0; i < gifAnimation->count; ++i) {
        SDL_FreeSurface(gifAnimation->frames[i]);
    }
    SDL_free(gifAnimation);

    IMG_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
