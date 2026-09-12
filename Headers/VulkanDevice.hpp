#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <stdexcept>

class VulkanDevice {
public:
    // The constructor takes the window and handles all initialization
    VulkanDevice(SDL_Window* window);
    
    // The destructor automatically cleans up Vulkan handles
    ~VulkanDevice();

    // Prevent copying to avoid double-freeing Vulkan resources
    VulkanDevice(const VulkanDevice&) = delete;
    VulkanDevice& operator=(const VulkanDevice&) = delete;

    // Getters for the rest of the engine to use
    VkInstance getInstance() const { return instance; }
    VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }
    VkDevice getDevice() const { return device; }
    VkSurfaceKHR getSurface() const { return surface; }
    VkQueue getGraphicsQueue() const { return graphicsQueue; }
    VkQueue getPresentQueue() const { return presentQueue; }
    
    uint32_t getGraphicsFamilyIndex() const { return graphicsFamilyIndex; }
    uint32_t getPresentFamilyIndex() const { return presentFamilyIndex; }

private:
    void createInstance(SDL_Window* window);
    void pickPhysicalDevice();
    void createLogicalDevice();

    VkInstance instance = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;
    
    uint32_t graphicsFamilyIndex = UINT32_MAX;
    uint32_t presentFamilyIndex = UINT32_MAX;
};
