#include "camel/VulkanDepthBuffer.hpp"

#include "camel/Logger.hpp"

#include <stdexcept>
#include <vector>

VulkanDepthBuffer::VulkanDepthBuffer(
    const VulkanDevice& device,
    const VulkanSwapchain& swapchain
)
    : deviceCore(device) {
    format = findSupportedFormat(
        deviceCore.getPhysicalDevice(),
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent = {swapchain.getExtent().width, swapchain.getExtent().height, 1};
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(deviceCore.getDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create depth image");
    }

    VkMemoryRequirements requirements{};
    vkGetImageMemoryRequirements(deviceCore.getDevice(), image, &requirements);

    VkMemoryAllocateInfo allocationInfo{};
    allocationInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocationInfo.allocationSize = requirements.size;
    allocationInfo.memoryTypeIndex = findMemoryType(
        requirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    if (vkAllocateMemory(
            deviceCore.getDevice(),
            &allocationInfo,
            nullptr,
            &memory
        ) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate depth image memory");
    }
    if (vkBindImageMemory(deviceCore.getDevice(), image, memory, 0) != VK_SUCCESS) {
        throw std::runtime_error("Failed to bind depth image memory");
    }

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    if (hasStencilComponent(format)) {
        viewInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(
            deviceCore.getDevice(), &viewInfo, nullptr, &imageView
        ) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create depth image view");
    }

    Logger::log(Logger::LogLevel::Info, "Depth buffer created.");
}

VulkanDepthBuffer::~VulkanDepthBuffer() {
    const VkDevice device = deviceCore.getDevice();
    if (imageView != VK_NULL_HANDLE) vkDestroyImageView(device, imageView, nullptr);
    if (image != VK_NULL_HANDLE) vkDestroyImage(device, image, nullptr);
    if (memory != VK_NULL_HANDLE) vkFreeMemory(device, memory, nullptr);
}

VkFormat VulkanDepthBuffer::findSupportedFormat(
    VkPhysicalDevice physicalDevice,
    const std::vector<VkFormat>& candidates,
    VkImageTiling tiling,
    VkFormatFeatureFlags features
) {
    for (VkFormat format : candidates) {
        VkFormatProperties properties{};
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &properties);
        const VkFormatFeatureFlags available =
            tiling == VK_IMAGE_TILING_LINEAR
                ? properties.linearTilingFeatures
                : properties.optimalTilingFeatures;
        if ((available & features) == features) {
            return format;
        }
    }
    throw std::runtime_error("Failed to find a supported Vulkan depth format");
}

uint32_t VulkanDepthBuffer::findMemoryType(
    uint32_t typeFilter,
    VkMemoryPropertyFlags properties
) const {
    VkPhysicalDeviceMemoryProperties memoryProperties{};
    vkGetPhysicalDeviceMemoryProperties(
        deviceCore.getPhysicalDevice(), &memoryProperties
    );
    for (uint32_t index = 0; index < memoryProperties.memoryTypeCount; ++index) {
        if ((typeFilter & (1u << index)) != 0 &&
            (memoryProperties.memoryTypes[index].propertyFlags & properties) == properties) {
            return index;
        }
    }
    throw std::runtime_error("Failed to find suitable Vulkan memory type");
}

bool VulkanDepthBuffer::hasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
           format == VK_FORMAT_D24_UNORM_S8_UINT ||
           format == VK_FORMAT_D16_UNORM_S8_UINT;
}
