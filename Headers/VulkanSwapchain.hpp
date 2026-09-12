#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "VulkanDevice.hpp"

class VulkanSwapchain {
public:
    VulkanSwapchain(const VulkanDevice& deviceCore);
    ~VulkanSwapchain();

    // Prevent copying
    VulkanSwapchain(const VulkanSwapchain&) = delete;
    VulkanSwapchain& operator=(const VulkanSwapchain&) = delete;

    VkSwapchainKHR getSwapchain() const { return swapchain; }
    VkFormat getImageFormat() const { return swapchainImageFormat; }
    VkExtent2D getExtent() const { return swapchainExtent; }
    const std::vector<VkImageView>& getImageViews() const { return swapchainImageViews; }
    size_t getImageCount() const { return swapchainImages.size(); }

private:
    void createSwapchain();
    void createImageViews();
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

    const VulkanDevice& deviceCore;
    
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkFormat swapchainImageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D swapchainExtent = { 0, 0 };
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;
};
