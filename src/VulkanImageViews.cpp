#include "camel/VulkanImageViews.hpp"

#include "camel/Logger.hpp"

#include <stdexcept>

VulkanImageViews::VulkanImageViews(
    const VulkanDevice& device,
    const VulkanSwapchain& swapchain
)
    : deviceCore(device), swapchainCore(swapchain) {
    imageViews.reserve(swapchainCore.getImages().size());
    try {
        for (VkImage image : swapchainCore.getImages()) {
            imageViews.push_back(createImageView(image, swapchainCore.getImageFormat()));
        }
    } catch (...) {
        for (VkImageView view : imageViews) {
            vkDestroyImageView(deviceCore.getDevice(), view, nullptr);
        }
        throw;
    }

    Logger::log(Logger::LogLevel::Info, "Swapchain image views created.");
}

VulkanImageViews::~VulkanImageViews() {
    for (VkImageView view : imageViews) {
        if (view != VK_NULL_HANDLE) {
            vkDestroyImageView(deviceCore.getDevice(), view, nullptr);
        }
    }
}

VkImageView VulkanImageViews::createImageView(VkImage image, VkFormat format) const {
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = image;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = format;
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    VkImageView view = VK_NULL_HANDLE;
    if (vkCreateImageView(deviceCore.getDevice(), &createInfo, nullptr, &view) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create swapchain image view");
    }
    return view;
}
