#include "camel/VulkanBuffer.hpp"

#include <cstring>
#include <stdexcept>

VulkanBuffer::VulkanBuffer(
    const VulkanDevice& device,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties
)
    : deviceCore(device) {
    if (size == 0) {
        throw std::invalid_argument("Cannot create a zero-sized Vulkan buffer");
    }

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(
            deviceCore.getDevice(), &bufferInfo, nullptr, &buffer
        ) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan buffer");
    }

    VkMemoryRequirements requirements{};
    vkGetBufferMemoryRequirements(deviceCore.getDevice(), buffer, &requirements);

    VkMemoryAllocateInfo allocationInfo{};
    allocationInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocationInfo.allocationSize = requirements.size;
    allocationInfo.memoryTypeIndex = findMemoryType(
        requirements.memoryTypeBits, properties
    );
    if (vkAllocateMemory(
            deviceCore.getDevice(), &allocationInfo, nullptr, &memory
        ) != VK_SUCCESS) {
        vkDestroyBuffer(deviceCore.getDevice(), buffer, nullptr);
        buffer = VK_NULL_HANDLE;
        throw std::runtime_error("Failed to allocate Vulkan buffer memory");
    }
    if (vkBindBufferMemory(deviceCore.getDevice(), buffer, memory, 0) != VK_SUCCESS) {
        vkFreeMemory(deviceCore.getDevice(), memory, nullptr);
        vkDestroyBuffer(deviceCore.getDevice(), buffer, nullptr);
        memory = VK_NULL_HANDLE;
        buffer = VK_NULL_HANDLE;
        throw std::runtime_error("Failed to bind Vulkan buffer memory");
    }
}

VulkanBuffer::~VulkanBuffer() {
    if (buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(deviceCore.getDevice(), buffer, nullptr);
    }
    if (memory != VK_NULL_HANDLE) {
        vkFreeMemory(deviceCore.getDevice(), memory, nullptr);
    }
}

void VulkanBuffer::mapMemory(const void* data, VkDeviceSize size) {
    if (data == nullptr || size == 0) {
        throw std::invalid_argument("Cannot map an empty Vulkan buffer upload");
    }

    void* mapped = nullptr;
    if (vkMapMemory(
            deviceCore.getDevice(), memory, 0, size, 0, &mapped
        ) != VK_SUCCESS) {
        throw std::runtime_error("Failed to map Vulkan buffer memory");
    }
    std::memcpy(mapped, data, static_cast<size_t>(size));
    vkUnmapMemory(deviceCore.getDevice(), memory);
}

uint32_t VulkanBuffer::findMemoryType(
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
