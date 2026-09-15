#include "camel/VulkanSwapchain.hpp"

#include "camel/Logger.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>

VulkanSwapchain::VulkanSwapchain(
    const VulkanDevice& device,
    const VulkanSurface& surface
)
    : deviceCore(device), surfaceCore(surface) {
    createSwapchain();
    Logger::log(Logger::LogLevel::Info, "Vulkan swapchain created.");
}

VulkanSwapchain::~VulkanSwapchain() {
    if (swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(deviceCore.getDevice(), swapchain, nullptr);
    }
}

VulkanSwapchain::SupportDetails VulkanSwapchain::querySupport() const {
    SupportDetails details;
    const VkPhysicalDevice physicalDevice = deviceCore.getPhysicalDevice();
    const VkSurfaceKHR surface = surfaceCore.get();

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
    details.formats.resize(formatCount);
    if (formatCount > 0) {
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            physicalDevice, surface, &formatCount, details.formats.data()
        );
    }

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        physicalDevice, surface, &presentModeCount, nullptr
    );
    details.presentModes.resize(presentModeCount);
    if (presentModeCount > 0) {
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            physicalDevice, surface, &presentModeCount, details.presentModes.data()
        );
    }

    return details;
}

VkSurfaceFormatKHR VulkanSwapchain::chooseSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& formats
) {
    if (formats.empty()) {
        throw std::invalid_argument("Cannot choose a surface format from an empty list");
    }

    if (formats.size() == 1 && formats.front().format == VK_FORMAT_UNDEFINED) {
        return {
            VK_FORMAT_B8G8R8A8_SRGB,
            VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
        };
    }

    for (const VkSurfaceFormatKHR& format : formats) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }
    return formats.front();
}

VkPresentModeKHR VulkanSwapchain::choosePresentMode(
    const std::vector<VkPresentModeKHR>& modes
) {
    for (VkPresentModeKHR mode : modes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return mode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanSwapchain::chooseExtent(
    const VkSurfaceCapabilitiesKHR& capabilities,
    SDL_Window* window
) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }
    if (window == nullptr) {
        throw std::invalid_argument("A window is required for an undefined swapchain extent");
    }

    int width = 0;
    int height = 0;
    if (!SDL_GetWindowSizeInPixels(window, &width, &height) || width <= 0 || height <= 0) {
        throw std::runtime_error("Failed to read drawable window size");
    }

    VkExtent2D chosen{
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };
    chosen.width = std::clamp(
        chosen.width,
        capabilities.minImageExtent.width,
        capabilities.maxImageExtent.width
    );
    chosen.height = std::clamp(
        chosen.height,
        capabilities.minImageExtent.height,
        capabilities.maxImageExtent.height
    );
    return chosen;
}

void VulkanSwapchain::createSwapchain() {
    const SupportDetails support = querySupport();
    if (support.formats.empty() || support.presentModes.empty()) {
        throw std::runtime_error("The selected surface has no usable swapchain formats or modes");
    }

    const VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(support.formats);
    const VkPresentModeKHR presentMode = choosePresentMode(support.presentModes);
    extent = chooseExtent(support.capabilities, surfaceCore.getWindow());

    uint32_t imageCount = support.capabilities.minImageCount + 1;
    if (support.capabilities.maxImageCount > 0) {
        imageCount = std::min(imageCount, support.capabilities.maxImageCount);
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surfaceCore.get();
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    const uint32_t familyIndices[] = {
        deviceCore.getGraphicsFamilyIndex(),
        deviceCore.getPresentFamilyIndex()
    };
    if (familyIndices[0] != familyIndices[1]) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = familyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = support.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    if (vkCreateSwapchainKHR(
            deviceCore.getDevice(),
            &createInfo,
            nullptr,
            &swapchain
        ) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan swapchain");
    }

    uint32_t actualImageCount = 0;
    vkGetSwapchainImagesKHR(deviceCore.getDevice(), swapchain, &actualImageCount, nullptr);
    images.resize(actualImageCount);
    vkGetSwapchainImagesKHR(
        deviceCore.getDevice(), swapchain, &actualImageCount, images.data()
    );
    imageFormat = surfaceFormat.format;
}
