#pragma once

#include "VulkanDevice.hpp"

#include <vulkan/vulkan.h>

class VulkanCommandExecutor {
public:
    explicit VulkanCommandExecutor(const VulkanDevice& device);
    ~VulkanCommandExecutor();

    VulkanCommandExecutor(const VulkanCommandExecutor&) = delete;
    VulkanCommandExecutor& operator=(const VulkanCommandExecutor&) = delete;

    VkCommandBuffer beginSingleTimeCommands() const;
    void endSingleTimeCommands(VkCommandBuffer commandBuffer) const;
    void copyBuffer(VkBuffer source, VkBuffer destination, VkDeviceSize size) const;

private:
    const VulkanDevice& deviceCore;
    VkCommandPool commandPool = VK_NULL_HANDLE;
};
