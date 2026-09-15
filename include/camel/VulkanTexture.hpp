#pragma once

#include "VulkanCommandExecutor.hpp"
#include "VulkanDevice.hpp"

#include <vulkan/vulkan.h>

#include <string>

class VulkanTexture {
public:
    VulkanTexture(
        const VulkanDevice& device,
        const std::string& filepath,
        const VulkanCommandExecutor& commandExecutor
    );
    ~VulkanTexture();

    VulkanTexture(const VulkanTexture&) = delete;
    VulkanTexture& operator=(const VulkanTexture&) = delete;

    VkImageView getImageView() const { return imageView; }
    VkSampler getSampler() const { return sampler; }

private:
    void transitionImageLayout(
        VkImageLayout oldLayout,
        VkImageLayout newLayout,
        const VulkanCommandExecutor& commandExecutor
    );
    void copyBufferToImage(
        VkBuffer buffer,
        uint32_t width,
        uint32_t height,
        const VulkanCommandExecutor& commandExecutor
    );
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

    const VulkanDevice& deviceCore;
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
};
