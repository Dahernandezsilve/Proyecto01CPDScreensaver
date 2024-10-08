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
    int posX, posY;
    int velX, velY;
};

// Variables globales para el audio
SDL_AudioSpec wavSpec;
Uint32 wavLength;
Uint8* wavBuffer;
SDL_AudioDeviceID audioDevice;

void setWindowIcon(SDL_Window* window, const char* iconPath) {
    SDL_Surface* iconSurface = IMG_Load(iconPath);
    if (!iconSurface) {
        cerr << "Failed to load icon image! SDL_image Error: " << IMG_GetError() << endl;
        return;
    }
    SDL_SetWindowIcon(window, iconSurface);
    SDL_FreeSurface(iconSurface);
}

void audioCallback(void* userdata, Uint8* stream, int len) {
    static Uint32 audioPosition = 0;
    if (audioPosition >= wavLength) return;
    Uint32 audioRemaining = wavLength - audioPosition;
    Uint32 audioLength = (len > audioRemaining) ? audioRemaining : len;
    SDL_memcpy(stream, wavBuffer + audioPosition, audioLength);
    audioPosition += audioLength;
}

SDL_Window* createWindow(const char* title, int width, int height) {
    SDL_Window* window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
    if (!window) {
        cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << endl;
    }
    return window;
}

SDL_Renderer* createRenderer(SDL_Window* window) {
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << endl;
        SDL_DestroyWindow(window);
    }
    return renderer;
}

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

void renderGIF(SDL_Renderer* renderer, const vector<SDL_Texture*>& gifTextures, IMG_Animation* gifAnimation, int x, int y, int w, int h, bool flip, int& frame, Uint32& frameTime) {
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - frameTime >= gifAnimation->delays[frame]) {
        frame = (frame + 1) % gifAnimation->count;
        frameTime = currentTime;
    }

    SDL_Rect dstRect = { x, y, w, h };
    SDL_RendererFlip flipType = flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

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

    if (SDL_LoadWAV("files/nyancatmusic.wav", &wavSpec, &wavBuffer, &wavLength) == NULL) {
        cerr << "Failed to load WAV file! SDL Error: " << SDL_GetError() << endl;
        SDL_Quit();
        return 1;
    }

    wavSpec.callback = audioCallback;
    wavSpec.userdata = NULL;

    audioDevice = SDL_OpenAudioDevice(NULL, 0, &wavSpec, NULL, 0);
    if (audioDevice == 0) {
        cerr << "Failed to open audio device! SDL Error: " << SDL_GetError() << endl;
        SDL_FreeWAV(wavBuffer);
        SDL_Quit();
        return 1;
    }

    SDL_Window* window = createWindow("Screen Saver", WIDTH, HEIGHT);
    if (!window) return 1;

    setWindowIcon(window, "files/codificacion.png");
    SDL_Cursor* cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
    SDL_SetCursor(cursor);
    
    SDL_Renderer* renderer = createRenderer(window);
    if (!renderer) return 1;

    IMG_Animation* gifAnimation = loadGIF("files/nyancat3.gif");
    if (!gifAnimation) return 1;

    vector<SDL_Texture*> gifTextures;
    gifTextures.resize(gifAnimation->count);

    for (int i = 0; i < gifAnimation->count; ++i) {
        gifTextures[i] = SDL_CreateTextureFromSurface(renderer, gifAnimation->frames[i]);
    }

    srand(time(nullptr));
    initializeGameOfLife(num_glider, num_gun, num_small_glider);


    vector<GIFInstance> gifs;
    vector<bool> flipFlags;

    gifs.push_back({100, 100, 5, 5});
    flipFlags.push_back(false);

    vector<int> frames(max_gifs, 0);
    vector<Uint32> frameTimes(max_gifs, SDL_GetTicks());

    bool running = true;
    SDL_Event e;

    Uint32 frameStart;
    int frameTime;

    while (running) {
        frameStart = SDL_GetTicks();
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                running = false;
            }
        }

        bool newGifAdded = false;

        #pragma omp parallel
        {
            #pragma omp for
            for (size_t i = 0; i < gifs.size(); ++i) {
                #pragma omp critical
                {
                    gifs[i].posX += gifs[i].velX;
                    gifs[i].posY += gifs[i].velY;

                    bool bounced = false;

                    if (gifs[i].posX <= 0 || gifs[i].posX + 120 >= WIDTH) {
                        gifs[i].velX = -gifs[i].velX;
                        flipFlags[i] = !flipFlags[i];
                        bounced = true;
                    }
                    if (gifs[i].posY <= 0 || gifs[i].posY + 30 >= HEIGHT) {
                        gifs[i].velY = -gifs[i].velY;
                        bounced = true;
                    }

                    if (bounced && gifs.size() < max_gifs && !newGifAdded) {
                        int newPosX = rand() % (WIDTH - 120);
                        int newPosY = rand() % (HEIGHT - 30);
                        int newVelX = (rand() % 7 + 1) * (rand() % 2 == 0 ? 1 : -1);
                        int newVelY = (rand() % 7 + 1) * (rand() % 2 == 0 ? 1 : -1);

                        gifs.push_back({newPosX, newPosY, newVelX, newVelY});
                        flipFlags.push_back(newVelX < 0);
                        frames.push_back(0);
                        frameTimes.push_back(SDL_GetTicks());
                        newGifAdded = true;
                    }
                }
            }
        }

        updateGameOfLife();

        SDL_RenderClear(renderer);
        renderBuffer(renderer); // Renderiza el Game of Life en el fondo

        #pragma omp parallel for
        for (size_t i = 0; i < gifs.size(); ++i) {
            #pragma omp critical
            {
                renderGIF(renderer, gifTextures, gifAnimation, gifs[i].posX, gifs[i].posY, 165, 65, flipFlags[i], frames[i], frameTimes[i]);
            }
        }

        SDL_RenderPresent(renderer);

        frameTime = SDL_GetTicks() - frameStart;

        if (frameTime < FRAME_TIME) {
            SDL_Delay(FRAME_TIME - frameTime);
        }

        frameCount++;
        if (SDL_GetTicks() - startTime >= 1000) {
            fps = frameCount / ((SDL_GetTicks() - startTime) / 1000.0f);
            frameCount = 0;
            startTime = SDL_GetTicks();
            frameCount = 0;

            // Actualiza el título de la ventana con los FPS
            char title[50];
            snprintf(title, sizeof(title), "[ScreenSaver - Parallel] - FPS: %.2f", fps);
            SDL_SetWindowTitle(window, title);
        }
    }

    SDL_CloseAudioDevice(audioDevice);
    SDL_FreeWAV(wavBuffer);

    for (SDL_Texture* texture : gifTextures) {
        SDL_DestroyTexture(texture);
    }

    IMG_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
