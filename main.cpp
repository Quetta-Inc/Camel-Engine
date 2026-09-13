#define SDL_MAIN_HANDLED

#include <SDL3/SDL.h>
#include <iostream>
#include <string>

#include "Headers/initialization.hpp"
#include "Headers/vulkan.hpp"
#include <string>

int main(int argc, char* argv[]) {
    const bool smokeTest = argc > 1 && std::string(argv[1]) == "--smoke-test";

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "Error to initialize SDL: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_WindowFlags windowFlags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;
    SDL_Window* window = SDL_CreateWindow(APP_NAME, WINDOW_WIDTH, WINDOW_HEIGHT, windowFlags);
    if (!window) {
        std::cerr << "Error to create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    VulkanContext vulkanContext{};

    if (!initVulkan(window, vulkanContext)) {
        std::cerr << "Failed to initialize Vulkan!" << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    if (smokeTest) {
        vkDeviceWaitIdle(vulkanContext.device);
        cleanupVulkan(vulkanContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }

    bool isRunning = true;
    SDL_Event event;

    if (smokeTest) {
        if (!drawFrame(vulkanContext)) {
            std::cerr << "Smoke test failed to draw a frame!" << std::endl;
            vkDeviceWaitIdle(vulkanContext.device);
            cleanupVulkan(vulkanContext);
            SDL_DestroyWindow(window);
            SDL_Quit();
            return -1;
        }
        isRunning = false;
    }

    std::cout << "Starting main application loop..." << std::endl;
    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                isRunning = false;
            }
        }
        if (vulkanContext.device != VK_NULL_HANDLE) {
            if (!drawFrame(vulkanContext)) {
                std::cerr << "Failed to draw frame!" << std::endl;
                break;
            }
        }
    }

    std::cout << "Cleaning up resources..." << std::endl;
    
    vkDeviceWaitIdle(vulkanContext.device); 
    cleanupVulkan(vulkanContext);
    
    SDL_DestroyWindow(window);
    SDL_Quit();
    std::cout << "Application successfully finished." << std::endl;
    return 0;
}
