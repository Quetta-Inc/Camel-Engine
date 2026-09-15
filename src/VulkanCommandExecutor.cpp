#include "camel/VulkanCommandExecutor.hpp"

#include <stdexcept>

VulkanCommandExecutor::VulkanCommandExecutor(const VulkanDevice& device)
    : deviceCore(device) {
    VkCommandPoolCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    createInfo.queueFamilyIndex = deviceCore.getGraphicsFamilyIndex();

    if (vkCreateCommandPool(
            deviceCore.getDevice(), &createInfo, nullptr, &commandPool
        ) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create transient command pool");
    }
}

VulkanCommandExecutor::~VulkanCommandExecutor() {
    if (commandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(deviceCore.getDevice(), commandPool, nullptr);
    }
}

VkCommandBuffer VulkanCommandExecutor::beginSingleTimeCommands() const {
    VkCommandBufferAllocateInfo allocationInfo{};
    allocationInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocationInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocationInfo.commandPool = commandPool;
    allocationInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    if (vkAllocateCommandBuffers(
            deviceCore.getDevice(), &allocationInfo, &commandBuffer
        ) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate transient command buffer");
    }

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        vkFreeCommandBuffers(deviceCore.getDevice(), commandPool, 1, &commandBuffer);
        throw std::runtime_error("Failed to begin transient command buffer");
    }
    return commandBuffer;
}

void VulkanCommandExecutor::endSingleTimeCommands(VkCommandBuffer commandBuffer) const {
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        vkFreeCommandBuffers(deviceCore.getDevice(), commandPool, 1, &commandBuffer);
        throw std::runtime_error("Failed to record transient command buffer");
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    if (vkQueueSubmit(deviceCore.getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
        vkFreeCommandBuffers(deviceCore.getDevice(), commandPool, 1, &commandBuffer);
        throw std::runtime_error("Failed to submit transient command buffer");
    }
    if (vkQueueWaitIdle(deviceCore.getGraphicsQueue()) != VK_SUCCESS) {
        vkFreeCommandBuffers(deviceCore.getDevice(), commandPool, 1, &commandBuffer);
        throw std::runtime_error("Failed while waiting for transient command buffer");
    }

    vkFreeCommandBuffers(deviceCore.getDevice(), commandPool, 1, &commandBuffer);
}

void VulkanCommandExecutor::copyBuffer(
    VkBuffer source,
    VkBuffer destination,
    VkDeviceSize size
) const {
    const VkCommandBuffer commandBuffer = beginSingleTimeCommands();
    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, source, destination, 1, &copyRegion);
    endSingleTimeCommands(commandBuffer);
}
