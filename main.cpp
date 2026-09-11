#define SDL_MAIN_HANDLED

#include <SDL3/SDL.h>
#include <iostream>

#include "Headers/initialization.hpp"
#include "Headers/vulkan.hpp"

int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "Критическая ошибка: SDL_Init сбой: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_WindowFlags windowFlags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;
    SDL_Window* window = SDL_CreateWindow("Hello, World!", WINDOW_WIDTH, WINDOW_HEIGHT, windowFlags);
    if (!window) {
        std::cerr << "Error to create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    VulkanContext vulkanContext{};

    if (!initVulkan(window, vulkanContext)) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    bool isRunning = true;
    SDL_Event event;

    std::cout << "Starting main application loop..." << std::endl;
    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                isRunning = false;
            }
        }

        if (vulkanContext.device != VK_NULL_HANDLE) {
            vkDeviceWaitIdle(vulkanContext.device);
        }
    }

    std::cout << "Cleaning up resources..." << std::endl;
    cleanupVulkan(vulkanContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    std::cout << "Application successfully finished." << std::endl;
    return 0;
}