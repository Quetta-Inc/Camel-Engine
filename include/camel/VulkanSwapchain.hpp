#pragma once

#include "VulkanDevice.hpp"
#include "VulkanSurface.hpp"

#include <vulkan/vulkan.h>

#include <vector>

class VulkanSwapchain {
public:
    VulkanSwapchain(const VulkanDevice& device, const VulkanSurface& surface);
    ~VulkanSwapchain();

    VulkanSwapchain(const VulkanSwapchain&) = delete;
    VulkanSwapchain& operator=(const VulkanSwapchain&) = delete;

    VkSwapchainKHR get() const { return swapchain; }
    VkFormat getImageFormat() const { return imageFormat; }
    VkExtent2D getExtent() const { return extent; }
    const std::vector<VkImage>& getImages() const { return images; }
    size_t getImageCount() const { return images.size(); }

    static VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
    static VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& modes);
    static VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities, SDL_Window* window);

private:
    struct SupportDetails {
        VkSurfaceCapabilitiesKHR capabilities{};
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    SupportDetails querySupport() const;
    void createSwapchain();

    const VulkanDevice& deviceCore;
    const VulkanSurface& surfaceCore;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkFormat imageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D extent{0, 0};
    std::vector<VkImage> images;
};
