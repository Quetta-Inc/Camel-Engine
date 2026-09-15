#pragma once

#include "VulkanInstance.hpp"
#include "VulkanSurface.hpp"

#include <vulkan/vulkan.h>

#include <optional>

struct QueueFamilyIndices {
    std::optional<uint32_t> graphics;
    std::optional<uint32_t> present;

    bool isComplete() const {
        return graphics.has_value() && present.has_value();
    }
};

class VulkanPhysicalDevice {
public:
    VulkanPhysicalDevice(const VulkanInstance& instance, const VulkanSurface& surface);
    ~VulkanPhysicalDevice() = default;

    VulkanPhysicalDevice(const VulkanPhysicalDevice&) = delete;
    VulkanPhysicalDevice& operator=(const VulkanPhysicalDevice&) = delete;

    VkPhysicalDevice get() const { return physicalDevice; }
    const QueueFamilyIndices& getQueueFamilies() const { return queueFamilies; }
    uint32_t getGraphicsFamilyIndex() const { return queueFamilies.graphics.value(); }
    uint32_t getPresentFamilyIndex() const { return queueFamilies.present.value(); }

    static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

private:
    static bool supportsRequiredExtensions(VkPhysicalDevice device);
    static bool isSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    QueueFamilyIndices queueFamilies;
};
