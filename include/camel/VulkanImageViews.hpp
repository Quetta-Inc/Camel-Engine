#pragma once

#include "VulkanDevice.hpp"
#include "VulkanSwapchain.hpp"

#include <vulkan/vulkan.h>

#include <vector>

class VulkanImageViews {
public:
    VulkanImageViews(const VulkanDevice& device, const VulkanSwapchain& swapchain);
    ~VulkanImageViews();

    VulkanImageViews(const VulkanImageViews&) = delete;
    VulkanImageViews& operator=(const VulkanImageViews&) = delete;

    const std::vector<VkImageView>& get() const { return imageViews; }
    size_t size() const { return imageViews.size(); }

private:
    VkImageView createImageView(VkImage image, VkFormat format) const;

    const VulkanDevice& deviceCore;
    const VulkanSwapchain& swapchainCore;
    std::vector<VkImageView> imageViews;
};
