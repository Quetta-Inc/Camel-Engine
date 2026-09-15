#define SDL_MAIN_HANDLED

#include "camel/ApplicationConfig.hpp"
#include "camel/Logger.hpp"
#include "camel/VulkanDevice.hpp"
#include "camel/VulkanInstance.hpp"
#include "camel/VulkanPhysicalDevice.hpp"
#include "camel/VulkanSurface.hpp"

#include <SDL3/SDL.h>

#include <exception>
#include <string>

int main(int argc, char* argv[]) {
    const bool smokeTest = argc > 1 && std::string(argv[1]) == "--smoke-test";

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        Logger::log(Logger::LogLevel::Error, SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        APP_NAME,
        static_cast<int>(WINDOW_WIDTH),
        static_cast<int>(WINDOW_HEIGHT),
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
    );
    if (window == nullptr) {
        Logger::log(Logger::LogLevel::Error, SDL_GetError());
        SDL_Quit();
        return 1;
    }

    int result = 0;
    try {
        VulkanInstance instance;
        VulkanSurface surface(instance, window);
        VulkanPhysicalDevice physicalDevice(instance, surface);
        VulkanDevice device(physicalDevice);

        if (!smokeTest) {
            bool running = true;
            SDL_Event event{};
            while (running) {
                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_EVENT_QUIT) {
                        running = false;
                    }
                }
                vkDeviceWaitIdle(device.getDevice());
            }
        } else {
            vkDeviceWaitIdle(device.getDevice());
        }
    } catch (const std::exception& exception) {
        Logger::log(Logger::LogLevel::Error, exception.what());
        result = 1;
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return result;
}
