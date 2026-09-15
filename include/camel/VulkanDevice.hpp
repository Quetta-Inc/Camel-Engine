#pragma once

#include "VulkanPhysicalDevice.hpp"

#include <vulkan/vulkan.h>

class VulkanDevice {
public:
    explicit VulkanDevice(const VulkanPhysicalDevice& physicalDevice);
    ~VulkanDevice();

    VulkanDevice(const VulkanDevice&) = delete;
    VulkanDevice& operator=(const VulkanDevice&) = delete;

    VkPhysicalDevice getPhysicalDevice() const { return physicalDeviceCore.get(); }
    VkDevice getDevice() const { return device; }
    VkQueue getGraphicsQueue() const { return graphicsQueue; }
    VkQueue getPresentQueue() const { return presentQueue; }
    uint32_t getGraphicsFamilyIndex() const { return physicalDeviceCore.getGraphicsFamilyIndex(); }
    uint32_t getPresentFamilyIndex() const { return physicalDeviceCore.getPresentFamilyIndex(); }

private:
    const VulkanPhysicalDevice& physicalDeviceCore;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;
};
