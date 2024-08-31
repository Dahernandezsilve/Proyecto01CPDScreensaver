#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include "gameofLife.h"

// Configuración de la ventana y renderizador
const int WIDTH = 800;
const int HEIGHT = 600;

// Estructura para representar cada GIF en la pantalla
struct GIFInstance {
    int posX, posY;
    int velX, velY;
};

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
void renderGIF(SDL_Renderer* renderer, IMG_Animation* gifAnimation, int x, int y, int w, int h, bool flip) {
    static int frame = 0;
    static Uint32 frameTime = SDL_GetTicks();

    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - frameTime >= gifAnimation->delays[frame]) {
        frame = (frame + 1) % gifAnimation->count;
        frameTime = currentTime;
    }

    SDL_Rect dstRect = { x, y, w, h };
    SDL_Texture* frameTexture = SDL_CreateTextureFromSurface(renderer, gifAnimation->frames[frame]);

    // Usar SDL_RendererFlip para voltear la imagen si es necesario
    SDL_RendererFlip flipType = flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

    SDL_RenderCopyEx(renderer, frameTexture, NULL, &dstRect, 0, NULL, flipType);
    SDL_DestroyTexture(frameTexture);
}


int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <max_gifs>" << std::endl;
        return 1;
    }

    int max_gifs = std::atoi(argv[1]);

    if (max_gifs <= 0) {
        std::cerr << "The number of GIFs must be greater than 0." << std::endl;
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = createWindow("Screen Saver", WIDTH, HEIGHT);
    if (!window) return 1;

    SDL_Renderer* renderer = createRenderer(window);
    if (!renderer) return 1;

    IMG_Animation* gifAnimation = loadGIF("files/nyancat3.gif");
    if (!gifAnimation) return 1;

    std::srand(std::time(nullptr));
    initializeGameOfLife();

    // Lista para almacenar todas las instancias de GIFs
    std::vector<GIFInstance> gifs;

    // Lista para almacenar si la imagen está volteada
    std::vector<bool> flipFlags;

    // Crear la primera instancia de GIF
    gifs.push_back({100, 100, 5, 5});
    flipFlags.push_back(false);

    bool running = true;
    SDL_Event e;

    while (running) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                running = false;
            }
        }

        // Bandera para controlar si se ha añadido un nuevo GIF en este frame
        bool newGifAdded = false;

        // Actualizar las posiciones de todos los GIFs
        for (size_t i = 0; i < gifs.size(); ++i) {
            gifs[i].posX += gifs[i].velX;
            gifs[i].posY += gifs[i].velY;

            bool bounced = false;

            // Rebotar en los bordes de la ventana
            if (gifs[i].posX <= 0 || gifs[i].posX + 165 >= WIDTH) {
                gifs[i].velX = -gifs[i].velX;
                flipFlags[i] = !flipFlags[i]; // Voltear la imagen horizontalmente
                bounced = true;
            }
            if (gifs[i].posY <= 0 || gifs[i].posY + 65 >= HEIGHT) {
                gifs[i].velY = -gifs[i].velY;
                bounced = true;
            }

            // Si ha rebotado, aún no hemos alcanzado el máximo de GIFs, y no se ha añadido un nuevo GIF en este frame
            if (bounced && gifs.size() < max_gifs && !newGifAdded) {
                int newPosX = std::rand() % (WIDTH - 165);
                int newPosY = std::rand() % (HEIGHT - 65);
                int newVelX = (std::rand() % 7 + 1) * (std::rand() % 2 == 0 ? 1 : -1);
                int newVelY = (std::rand() % 7 + 1) * (std::rand() % 2 == 0 ? 1 : -1);

                gifs.push_back({newPosX, newPosY, newVelX, newVelY});
                flipFlags.push_back(newVelX < 0); // Determina si el nuevo GIF debe estar volteado inicialmente
                newGifAdded = true; // Marcar que se ha añadido un GIF en este frame
            }
        }

        updateGameOfLife();

        SDL_RenderClear(renderer);
        renderBuffer(renderer); // Renderiza el Game of Life en el fondo

        // Renderizar todos los GIFs
        for (size_t i = 0; i < gifs.size(); ++i) {
            renderGIF(renderer, gifAnimation, gifs[i].posX, gifs[i].posY, 165, 65, flipFlags[i]);
        }

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