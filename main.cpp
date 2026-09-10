#include <iostream>

#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>

int main() {

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }

    // FIXME: use SDL_WINDOW_VULKAN
    SDL_Window* window = SDL_CreateWindow("Camel Engine", 800, 600, NULL);

    if (!window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return EXIT_FAILURE;
    }

    bool isRunning = true;
    SDL_Event event;

    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                isRunning = false;
            }
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;

}
