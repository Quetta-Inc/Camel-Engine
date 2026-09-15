#include "camel/VulkanSurface.hpp"

#include "camel/Logger.hpp"

#include <stdexcept>
#include <string>

VulkanSurface::VulkanSurface(const VulkanInstance& instance, SDL_Window* window)
    : instanceCore(instance), window(window) {
    if (window == nullptr) {
        throw std::invalid_argument("Cannot create a Vulkan surface without an SDL window");
    }

    if (!SDL_Vulkan_CreateSurface(window, instanceCore.get(), nullptr, &surface)) {
        throw std::runtime_error(
            std::string("Failed to create Vulkan surface: ") + SDL_GetError()
        );
    }

    Logger::log(Logger::LogLevel::Info, "Vulkan surface created.");
}

VulkanSurface::~VulkanSurface() {
    if (surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(instanceCore.get(), surface, nullptr);
    }
}
