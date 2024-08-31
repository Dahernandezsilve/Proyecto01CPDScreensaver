#include <iostream>
#include <cstring>
#include <vector>
#include <random>
#include <ctime>

struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

const int FRAMEBUFFER_WIDTH = 200;
const int FRAMEBUFFER_HEIGHT = 200;
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;
const float RENDER_SCALE = 0.95;
const int RENDER_WIDTH = FRAMEBUFFER_WIDTH / RENDER_SCALE;
const int RENDER_HEIGHT = FRAMEBUFFER_HEIGHT / RENDER_SCALE;
const int FRAMEBUFFER_SIZE = RENDER_WIDTH * RENDER_HEIGHT;

Color framebuffer[FRAMEBUFFER_SIZE];
Color aliveColor = { 255, 255, 255 };
Color deadColor = { 0, 0, 0 };

void setPixel(int x, int y, Color color) {
    if (x >= 0 && x < RENDER_WIDTH && y >= 0 && y < RENDER_HEIGHT) {
        framebuffer[y * RENDER_WIDTH + x] = color;
    }
}

void renderBuffer(SDL_Renderer* renderer) {
    // Crear una textura
    SDL_Texture* texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_ABGR8888,
            SDL_TEXTUREACCESS_STATIC,
            RENDER_WIDTH,
            RENDER_HEIGHT
    );

    // Actualizar la textura con los datos de los píxeles en el framebuffer
    SDL_UpdateTexture(
            texture,
            NULL,
            framebuffer,
            RENDER_WIDTH * sizeof(Color)
    );

    // Limpiar el renderer
    SDL_RenderClear(renderer);

    // Establecer la escala para la representación
    SDL_RenderSetScale(renderer, RENDER_SCALE, RENDER_SCALE);

    // Copiar la textura al renderer
    SDL_RenderCopy(renderer, texture, NULL, NULL);

    // Destruir la textura
    SDL_DestroyTexture(texture);

    // Actualizar el renderer
    SDL_RenderPresent(renderer);
}

int countAliveNeighbors(int x, int y) {
    int aliveNeighbors = 0;

    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 0 && j == 0) {
                continue;
            }

            int neighborX = x + i;
            int neighborY = y + j;

            if (neighborX >= 0 && neighborX < RENDER_WIDTH && neighborY >= 0 && neighborY < RENDER_HEIGHT) {
                if (framebuffer[neighborY * RENDER_WIDTH + neighborX].r == aliveColor.r &&
                    framebuffer[neighborY * RENDER_WIDTH + neighborX].g == aliveColor.g &&
                    framebuffer[neighborY * RENDER_WIDTH + neighborX].b == aliveColor.b) {
                    aliveNeighbors++;
                }
            }
        }
    }

    return aliveNeighbors;
}

void updateGameOfLife() {
    Color newFramebuffer[FRAMEBUFFER_SIZE];

    for (int y = 0; y < RENDER_HEIGHT; y++) {
        for (int x = 0; x < RENDER_WIDTH; x++) {
            Color currentColor = framebuffer[y * RENDER_WIDTH + x];
            bool isAlive = (currentColor.r == aliveColor.r &&
                            currentColor.g == aliveColor.g &&
                            currentColor.b == aliveColor.b);

            int aliveNeighbors = countAliveNeighbors(x, y);

            // Aplicar las reglas del Juego de la Vida
            if (isAlive) {
                if (aliveNeighbors < 2 || aliveNeighbors > 3) {
                    newFramebuffer[y * RENDER_WIDTH + x] = deadColor;
                } else {
                    newFramebuffer[y * RENDER_WIDTH + x] = aliveColor;
                }
            } else {
                if (aliveNeighbors == 3) {
                    newFramebuffer[y * RENDER_WIDTH + x] = aliveColor;
                } else {
                    newFramebuffer[y * RENDER_WIDTH + x] = deadColor;
                }
            }
        }
    }

    std::memcpy(framebuffer, newFramebuffer, sizeof(framebuffer));
}

void initializeGameOfLife() {
    std::memset(framebuffer, 0, sizeof(framebuffer));

    std::vector<std::pair<int, int>> gliderPattern = {
            { 1, 0 }, { 2, 1 }, { 0, 2 }, { 1, 2 }, { 2, 2 }
    };

    for (int i = 0; i < 30; i++) {  // Aumentar la cantidad de gliders
        int x = std::rand() % (RENDER_WIDTH - 5);
        int y = std::rand() % (RENDER_HEIGHT - 5);

        for (const auto& point : gliderPattern) {
            int gliderX = point.first + x;
            int gliderY = point.second + y;
            setPixel(gliderX, gliderY, aliveColor);
        }
    }

    std::vector<std::pair<int, int>> gunPattern = {
            { 0, 4 }, { 1, 4 }, { 0, 5 }, { 1, 5 }, { 10, 4 }, { 10, 5 }, { 10, 6 },
            { 11, 3 }, { 11, 7 }, { 12, 2 }, { 12, 8 }, { 13, 2 }, { 13, 8 }, { 14, 5 },
            { 15, 3 }, { 15, 7 }, { 16, 4 }, { 16, 5 }, { 16, 6 }, { 17, 5 }, { 20, 2 },
            { 20, 3 }, { 20, 4 }, { 21, 2 }, { 21, 3 }, { 21, 4 }, { 22, 1 }, { 22, 5 },
            { 24, 0 }, { 24, 1 }, { 24, 5 }, { 24, 6 }, { 34, 2 }, { 34, 3 }, { 35, 2 },
            { 35, 3 }
    };

    for (int i = 0; i < 15; i++) {  // Aumentar la cantidad de guns
        int x = std::rand() % (RENDER_WIDTH - 35);
        int y = std::rand() % (RENDER_HEIGHT - 10);

        for (const auto& point : gunPattern) {
            int gunX = point.first + x;
            int gunY = point.second + y;
            setPixel(gunX, gunY, aliveColor);
        }
    }

    std::vector<std::pair<int, int>> smallGliderPattern = {
            { 0, 0 }, { 1, 0 }, { 2, 0 }, { 0, 1 }, { 1, 2 }
    };

    for (int i = 0; i < 50; i++) {  // Mantener la cantidad de small gliders
        int x = std::rand() % (RENDER_WIDTH - 2);
        int y = std::rand() % (RENDER_HEIGHT - 2);

        for (const auto& point : smallGliderPattern) {
            int gliderX = point.first + x;
            int gliderY = point.second + y;
            setPixel(gliderX, gliderY, aliveColor);
        }
    }
}