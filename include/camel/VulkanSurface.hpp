#pragma once

#include "VulkanInstance.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.h>

class VulkanSurface {
public:
    VulkanSurface(const VulkanInstance& instance, SDL_Window* window);
    ~VulkanSurface();

    VulkanSurface(const VulkanSurface&) = delete;
    VulkanSurface& operator=(const VulkanSurface&) = delete;

    VkSurfaceKHR get() const { return surface; }
    SDL_Window* getWindow() const { return window; }

private:
    const VulkanInstance& instanceCore;
    SDL_Window* window = nullptr;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
};
