#include "camel/VulkanPhysicalDevice.hpp"

#include "camel/Logger.hpp"

#include <array>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

constexpr std::array<const char*, 1> requiredDeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

} // namespace

VulkanPhysicalDevice::VulkanPhysicalDevice(
    const VulkanInstance& instance,
    const VulkanSurface& surface
) {
    uint32_t deviceCount = 0;
    if (vkEnumeratePhysicalDevices(instance.get(), &deviceCount, nullptr) != VK_SUCCESS ||
        deviceCount == 0) {
        throw std::runtime_error("No Vulkan-capable physical devices were found");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    if (vkEnumeratePhysicalDevices(instance.get(), &deviceCount, devices.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to enumerate Vulkan physical devices");
    }

    for (VkPhysicalDevice candidate : devices) {
        if (isSuitable(candidate, surface.get())) {
            physicalDevice = candidate;
            queueFamilies = findQueueFamilies(candidate, surface.get());
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error(
            "No suitable Vulkan device with graphics, present, and swapchain support was found"
        );
    }

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    Logger::log(
        Logger::LogLevel::Info,
        std::string("Using Vulkan device: ") + properties.deviceName
    );
}

QueueFamilyIndices VulkanPhysicalDevice::findQueueFamilies(
    VkPhysicalDevice device,
    VkSurfaceKHR surface
) {
    QueueFamilyIndices indices;
    uint32_t familyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, nullptr);
    std::vector<VkQueueFamilyProperties> families(familyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, families.data());

    for (uint32_t index = 0; index < familyCount; ++index) {
        if ((families[index].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
            indices.graphics = index;
        }

        VkBool32 presentSupport = VK_FALSE;
        if (vkGetPhysicalDeviceSurfaceSupportKHR(device, index, surface, &presentSupport) == VK_SUCCESS &&
            presentSupport == VK_TRUE) {
            indices.present = index;
        }

        if (indices.isComplete()) {
            break;
        }
    }

    return indices;
}

bool VulkanPhysicalDevice::supportsRequiredExtensions(VkPhysicalDevice device) {
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> available(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, available.data());

    std::set<std::string> required(
        requiredDeviceExtensions.begin(),
        requiredDeviceExtensions.end()
    );
    for (const VkExtensionProperties& extension : available) {
        required.erase(extension.extensionName);
    }
    return required.empty();
}

bool VulkanPhysicalDevice::isSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
    if (!supportsRequiredExtensions(device)) {
        return false;
    }

    const QueueFamilyIndices indices = findQueueFamilies(device, surface);
    if (!indices.isComplete()) {
        return false;
    }

    uint32_t formatCount = 0;
    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    return formatCount > 0 && presentModeCount > 0;
}
