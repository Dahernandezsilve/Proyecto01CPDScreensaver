#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_audio.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <omp.h>
#include "gameofLife.h"

using namespace std;

// Variables para FPS
Uint32 startTime = 0;
int frameCount = 0;
float fps = 0.0f;
const int FRAME_TIME = 1000 / 60;

// Configuración de la ventana y renderizador
const int WIDTH = 800;
const int HEIGHT = 600;

// Estructura para representar cada GIF en la pantalla
struct GIFInstance {
    float posX, posY;
    float velX, velY;
    bool flipped;
};

// Variables globales para el audio
SDL_AudioSpec wavSpec;
Uint32 wavLength;
Uint8* wavBuffer;
SDL_AudioDeviceID audioDevice;

SDL_Texture* gameOfLifeTexture = nullptr;

void updateGameOfLifeTexture(SDL_Renderer* renderer) {
    SDL_UpdateTexture(gameOfLifeTexture, NULL, framebuffer, RENDER_WIDTH * sizeof(Color));
}

void setWindowIcon(SDL_Window* window, const char* iconPath) {
    // Cargar la imagen del icono
    SDL_Surface* iconSurface = IMG_Load(iconPath);
    if (!iconSurface) {
        cerr << "Failed to load icon image! SDL_image Error: " << IMG_GetError() << endl;
        return;
    }

    // Establecer la imagen del icono para la ventana
    SDL_SetWindowIcon(window, iconSurface);

    // Liberar la superficie del icono
    SDL_FreeSurface(iconSurface);
}

// Callback de audio para manejar la reproducción
void audioCallback(void* userdata, Uint8* stream, int len) {
    static Uint32 audioPosition = 0; // Posición actual en el buffer de audio

    if (audioPosition >= wavLength) { 
        return; // No más audio para reproducir
    }

    // Ajustar la longitud si el audio restante es menor que el tamaño de stream
    Uint32 audioRemaining = wavLength - audioPosition;
    Uint32 audioLength = (len > audioRemaining) ? audioRemaining : len;

    // Copiar los datos de audio al stream
    SDL_memcpy(stream, wavBuffer + audioPosition, audioLength);

    // Avanzar la posición
    audioPosition += audioLength;
}

// Función para crear la ventana
SDL_Window* createWindow(const char* title, int width, int height) {
    SDL_Window* window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
    if (!window) {
        cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << endl;
    }
    return window;
}

// Función para crear el renderizador
SDL_Renderer* createRenderer(SDL_Window* window) {
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << endl;
        SDL_DestroyWindow(window);
    }
    return renderer;
}

// Función para cargar el GIF
IMG_Animation* loadGIF(const char* filePath) {
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << endl;
        return nullptr;
    }

    SDL_RWops* rwop = SDL_RWFromFile(filePath, "rb");
    if (!rwop) {
        cerr << "Failed to load GIF! SDL Error: " << SDL_GetError() << endl;
        IMG_Quit();
        return nullptr;
    }

    IMG_Animation* gifAnimation = IMG_LoadGIFAnimation_RW(rwop);
    SDL_RWclose(rwop);

    if (!gifAnimation) {
        cerr << "Failed to load GIF animation! SDL_image Error: " << IMG_GetError() << endl;
        IMG_Quit();
        return nullptr;
    }

    return gifAnimation;
}

void renderGIF(SDL_Renderer* renderer, const vector<SDL_Texture*>& gifTextures, IMG_Animation* gifAnimation, const GIFInstance& gif) {
    static int frame = 0;
    static Uint32 lastFrameTime = SDL_GetTicks();
    Uint32 currentTime = SDL_GetTicks();

    Uint32 deltaTime = currentTime - lastFrameTime;

    if (deltaTime >= gifAnimation->delays[frame]) {
        frame = (frame + 1) % gifAnimation->count;
        lastFrameTime = currentTime;
    }

    SDL_Rect dstRect = { static_cast<int>(gif.posX), static_cast<int>(gif.posY), 160, 60 };
    SDL_RendererFlip flipType = gif.flipped ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

    SDL_RenderCopyEx(renderer, gifTextures[frame], NULL, &dstRect, 0, NULL, flipType);
}


int main(int argc, char* argv[]) {
    if (argc < 4) {
        cerr << "Usage: " << argv[0] << " <max_gifs> <num_glider> <num_gun> <num_smallGlider>" << endl;
        return 1;
    }

    int max_gifs = atoi(argv[1]);
    int num_glider = atoi(argv[2]);
    int num_gun = atoi(argv[3]);
    int num_small_glider = atoi(argv[4]);

    if (max_gifs <= 0) {
        cerr << "The number of GIFs must be greater than 0." << endl;
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Screen Saver", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << endl;
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << endl;
        SDL_DestroyWindow(window);
        return 1;
    }

    // Cargar GIF y crear texturas
    IMG_Animation* gifAnimation = loadGIF("files/nyancat3.gif");
    if (!gifAnimation) {
        cerr << "Failed to load GIF! SDL_image Error: " << IMG_GetError() << endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return 1;
    }

    vector<SDL_Texture*> gifTextures;
    for (int i = 0; i < gifAnimation->count; ++i) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, gifAnimation->frames[i]);
        gifTextures.push_back(texture);
    }

    vector<GIFInstance> gifs;
    gifs.reserve(max_gifs);

    gifs.push_back({100.0f, 100.0f, 5.0f, 5.0f, false});

    // Inicializar Game of Life
    initializeGameOfLife(num_glider, num_gun, num_small_glider);

    // Crear textura para el Game of Life
    gameOfLifeTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, RENDER_WIDTH, RENDER_HEIGHT);
    if (!gameOfLifeTexture) {
        cerr << "Failed to create Game of Life texture! SDL Error: " << SDL_GetError() << endl;
        // Manejar el error apropiadamente
    }

    bool running = true;
    SDL_Event e;

    Uint32 frameStart;
    int frameTime;
    Uint32 totalExecutionTime = 0;

    while (running) {
        frameStart = SDL_GetTicks();
        
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                running = false;
            }
        }

        // Añadir nuevos GIFs si un GIF rebota
        bool gifAdded = false;

        #pragma omp parallel for schedule(dynamic)
        for (size_t i = 0; i < gifs.size(); ++i) {
            gifs[i].posX += gifs[i].velX;
            gifs[i].posY += gifs[i].velY;

            // Rebotar en los bordes de la ventana
            bool rebote = false;
            if (gifs[i].posX <= 0 || gifs[i].posX + 120 >= WIDTH) {
                gifs[i].velX = -gifs[i].velX;
                gifs[i].flipped = !gifs[i].flipped;
                rebote = true;
            }
            if (gifs[i].posY <= 0 || gifs[i].posY + 30 >= HEIGHT) {
                gifs[i].velY = -gifs[i].velY;
                rebote = true;
            }

            // Si hubo rebote y no se ha añadido un GIF en este ciclo, añadir uno nuevo
            if (rebote && !gifAdded && gifs.size() < max_gifs) {
                #pragma omp critical
                {
                    if (!gifAdded) { // Reconfirmamos dentro de la sección crítica
                        float newPosX = static_cast<float>(rand() % (WIDTH - 120));
                        float newPosY = static_cast<float>(rand() % (HEIGHT - 30));
                        float newVelX = static_cast<float>((rand() % 7 + 1) * (rand() % 2 == 0 ? 1 : -1));
                        float newVelY = static_cast<float>((rand() % 7 + 1) * (rand() % 2 == 0 ? 1 : -1));
                        gifs.push_back({newPosX, newPosY, newVelX, newVelY, newVelX < 0});
                        gifAdded = true; // Evitar añadir más de un GIF en este ciclo
                    }
                }
            }
}

        updateGameOfLife();
        updateGameOfLifeTexture(renderer);

        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderClear(renderer);
        
        // Renderizar Game of Life
        SDL_RenderCopy(renderer, gameOfLifeTexture, NULL, NULL);

        // Renderizar todos los GIFs
        for (const auto& gif : gifs) {
            renderGIF(renderer, gifTextures, gifAnimation, gif);
        }

        SDL_RenderPresent(renderer);

        frameTime = SDL_GetTicks() - frameStart;
        totalExecutionTime += frameTime;

        if (frameTime < FRAME_TIME) {
            SDL_Delay(FRAME_TIME - frameTime);
        }

        frameCount++;
        if (SDL_GetTicks() - startTime >= 1000) {
            fps = frameCount / ((SDL_GetTicks() - startTime) / 1000.0f);
            startTime = SDL_GetTicks();
            frameCount = 0;

            char title[50];
            snprintf(title, sizeof(title), "[ScreenSaver - Parallel] - FPS: %.2f", fps);
            SDL_SetWindowTitle(window, title);
        }
    }

    cout << "Total Execution Time: " << totalExecutionTime << " ms" << endl;

    // Limpiar recursos
    for (auto texture : gifTextures) {
        SDL_DestroyTexture(texture);
    }
    SDL_DestroyTexture(gameOfLifeTexture);
    IMG_FreeAnimation(gifAnimation);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}