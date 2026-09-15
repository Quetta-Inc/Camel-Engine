#include "camel/VulkanTexture.hpp"

#include "camel/VulkanBuffer.hpp"
#include "camel/Logger.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <stdexcept>
#include <string>

VulkanTexture::VulkanTexture(
    const VulkanDevice& device,
    const std::string& filepath,
    const VulkanCommandExecutor& commandExecutor
)
    : deviceCore(device) {
    int width = 0;
    int height = 0;
    int channels = 0;
    stbi_uc* pixels = stbi_load(
        filepath.c_str(), &width, &height, &channels, STBI_rgb_alpha
    );
    if (pixels == nullptr) {
        const char* reason = stbi_failure_reason();
        throw std::runtime_error(
            "Failed to load texture '" + filepath + "': " +
            (reason == nullptr ? "unknown reason" : reason)
        );
    }

    try {
        const VkDeviceSize imageSize =
            static_cast<VkDeviceSize>(width) *
            static_cast<VkDeviceSize>(height) * 4;
        VulkanBuffer stagingBuffer(
            deviceCore,
            imageSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );
        stagingBuffer.mapMemory(pixels, imageSize);
        stbi_image_free(pixels);
        pixels = nullptr;

        constexpr VkFormat textureFormat = VK_FORMAT_R8G8B8A8_SRGB;
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent = {
            static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1
        };
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = textureFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        if (vkCreateImage(
                deviceCore.getDevice(), &imageInfo, nullptr, &image
            ) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create texture image");
        }

        VkMemoryRequirements requirements{};
        vkGetImageMemoryRequirements(deviceCore.getDevice(), image, &requirements);
        VkMemoryAllocateInfo allocationInfo{};
        allocationInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocationInfo.allocationSize = requirements.size;
        allocationInfo.memoryTypeIndex = findMemoryType(
            requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );
        if (vkAllocateMemory(
                deviceCore.getDevice(), &allocationInfo, nullptr, &memory
            ) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate texture memory");
        }
        if (vkBindImageMemory(deviceCore.getDevice(), image, memory, 0) != VK_SUCCESS) {
            throw std::runtime_error("Failed to bind texture memory");
        }

        transitionImageLayout(
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            commandExecutor
        );
        copyBufferToImage(
            stagingBuffer.getBuffer(),
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height),
            commandExecutor
        );
        transitionImageLayout(
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            commandExecutor
        );

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = textureFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.layerCount = 1;
        if (vkCreateImageView(
                deviceCore.getDevice(), &viewInfo, nullptr, &imageView
            ) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create texture image view");
        }

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        if (vkCreateSampler(
                deviceCore.getDevice(), &samplerInfo, nullptr, &sampler
            ) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create texture sampler");
        }
    } catch (...) {
        if (pixels != nullptr) stbi_image_free(pixels);
        if (sampler != VK_NULL_HANDLE) vkDestroySampler(deviceCore.getDevice(), sampler, nullptr);
        if (imageView != VK_NULL_HANDLE) vkDestroyImageView(deviceCore.getDevice(), imageView, nullptr);
        if (image != VK_NULL_HANDLE) vkDestroyImage(deviceCore.getDevice(), image, nullptr);
        if (memory != VK_NULL_HANDLE) vkFreeMemory(deviceCore.getDevice(), memory, nullptr);
        sampler = VK_NULL_HANDLE;
        imageView = VK_NULL_HANDLE;
        image = VK_NULL_HANDLE;
        memory = VK_NULL_HANDLE;
        throw;
    }

    Logger::log(Logger::LogLevel::Info, "Texture loaded: " + filepath);
}

VulkanTexture::~VulkanTexture() {
    const VkDevice device = deviceCore.getDevice();
    if (sampler != VK_NULL_HANDLE) vkDestroySampler(device, sampler, nullptr);
    if (imageView != VK_NULL_HANDLE) vkDestroyImageView(device, imageView, nullptr);
    if (image != VK_NULL_HANDLE) vkDestroyImage(device, image, nullptr);
    if (memory != VK_NULL_HANDLE) vkFreeMemory(device, memory, nullptr);
}

void VulkanTexture::transitionImageLayout(
    VkImageLayout oldLayout,
    VkImageLayout newLayout,
    const VulkanCommandExecutor& commandExecutor
) {
    VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    VkAccessFlags sourceAccess = 0;
    VkAccessFlags destinationAccess = VK_ACCESS_TRANSFER_WRITE_BIT;

    if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        sourceAccess = VK_ACCESS_TRANSFER_WRITE_BIT;
        destinationAccess = VK_ACCESS_SHADER_READ_BIT;
    } else if (!(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
                 newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)) {
        throw std::invalid_argument("Unsupported texture image layout transition");
    }

    const VkCommandBuffer commandBuffer = commandExecutor.beginSingleTimeCommands();
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.srcAccessMask = sourceAccess;
    barrier.dstAccessMask = destinationAccess;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.layerCount = 1;

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage,
        destinationStage,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &barrier
    );
    commandExecutor.endSingleTimeCommands(commandBuffer);
}

void VulkanTexture::copyBufferToImage(
    VkBuffer buffer,
    uint32_t width,
    uint32_t height,
    const VulkanCommandExecutor& commandExecutor
) {
    const VkCommandBuffer commandBuffer = commandExecutor.beginSingleTimeCommands();
    VkBufferImageCopy region{};
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageExtent = {width, height, 1};
    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );
    commandExecutor.endSingleTimeCommands(commandBuffer);
}

uint32_t VulkanTexture::findMemoryType(
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
