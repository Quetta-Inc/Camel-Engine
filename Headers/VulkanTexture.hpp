#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include "VulkanDevice.hpp"

class VulkanTexture {
public:
    VulkanTexture(const VulkanDevice& device, const std::string& filepath);
    ~VulkanTexture();

    VulkanTexture(const VulkanTexture&) = delete;
    VulkanTexture& operator=(const VulkanTexture&) = delete;

    VkImageView getImageView() const { return imageView; }
    VkSampler getSampler() const { return sampler; }

private:
    void transitionImageLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkCommandPool tempPool);
    void copyBufferToImage(VkBuffer buffer, uint32_t width, uint32_t height, VkCommandPool tempPool);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    const VulkanDevice& deviceCore;
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
};
