#pragma once

#include "VulkanDevice.hpp"
#include "VulkanSwapchain.hpp"

#include <vulkan/vulkan.h>

#include <vector>

class VulkanDepthBuffer {
public:
    VulkanDepthBuffer(const VulkanDevice& device, const VulkanSwapchain& swapchain);
    ~VulkanDepthBuffer();

    VulkanDepthBuffer(const VulkanDepthBuffer&) = delete;
    VulkanDepthBuffer& operator=(const VulkanDepthBuffer&) = delete;

    VkFormat getFormat() const { return format; }
    VkImageView getImageView() const { return imageView; }

    static VkFormat findSupportedFormat(
        VkPhysicalDevice physicalDevice,
        const std::vector<VkFormat>& candidates,
        VkImageTiling tiling,
        VkFormatFeatureFlags features
    );

private:
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
    static bool hasStencilComponent(VkFormat format);

    const VulkanDevice& deviceCore;
    VkFormat format = VK_FORMAT_UNDEFINED;
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;
};
