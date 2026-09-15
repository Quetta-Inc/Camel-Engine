#include "camel/VulkanDevice.hpp"

#include "camel/Logger.hpp"

#include <set>
#include <stdexcept>
#include <vector>

VulkanDevice::VulkanDevice(const VulkanPhysicalDevice& physicalDevice)
    : physicalDeviceCore(physicalDevice) {
    const float queuePriority = 1.0f;
    const std::set<uint32_t> uniqueFamilies = {
        physicalDeviceCore.getGraphicsFamilyIndex(),
        physicalDeviceCore.getPresentFamilyIndex()
    };

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    queueCreateInfos.reserve(uniqueFamilies.size());
    for (uint32_t family : uniqueFamilies) {
        VkDeviceQueueCreateInfo queueInfo{};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = family;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    const char* deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = 1;
    createInfo.ppEnabledExtensionNames = deviceExtensions;

    if (vkCreateDevice(
            physicalDeviceCore.get(),
            &createInfo,
            nullptr,
            &device
        ) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan logical device");
    }

    vkGetDeviceQueue(
        device,
        physicalDeviceCore.getGraphicsFamilyIndex(),
        0,
        &graphicsQueue
    );
    vkGetDeviceQueue(
        device,
        physicalDeviceCore.getPresentFamilyIndex(),
        0,
        &presentQueue
    );

    Logger::log(Logger::LogLevel::Info, "Vulkan logical device created.");
}

VulkanDevice::~VulkanDevice() {
    if (device != VK_NULL_HANDLE) {
        vkDestroyDevice(device, nullptr);
    }
}
