#pragma once

#include <vulkan/vulkan.h>

class VulkanInstance {
public:
    VulkanInstance();
    ~VulkanInstance();

    VulkanInstance(const VulkanInstance&) = delete;
    VulkanInstance& operator=(const VulkanInstance&) = delete;

    VkInstance get() const { return instance; }

private:
    VkInstance instance = VK_NULL_HANDLE;
};
