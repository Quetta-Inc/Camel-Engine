#include "camel/VulkanRenderer.hpp"

#include "camel/ApplicationConfig.hpp"
#include "camel/Logger.hpp"

#include <stdexcept>

VulkanRenderer::VulkanRenderer(
    const VulkanDevice& device,
    const VulkanSwapchain& swapchain
)
    : deviceCore(device), swapchainCore(swapchain) {
    createCommandPool();
    createCommandBuffers();
    createSyncObjects();
    Logger::log(Logger::LogLevel::Info, "Vulkan renderer initialized.");
}

VulkanRenderer::~VulkanRenderer() {
    const VkDevice device = deviceCore.getDevice();
    for (uint32_t index = 0; index < framesInFlight; ++index) {
        if (renderFinishedSemaphores[index] != VK_NULL_HANDLE) {
            vkDestroySemaphore(device, renderFinishedSemaphores[index], nullptr);
        }
        if (imageAvailableSemaphores[index] != VK_NULL_HANDLE) {
            vkDestroySemaphore(device, imageAvailableSemaphores[index], nullptr);
        }
        if (inFlightFences[index] != VK_NULL_HANDLE) {
            vkDestroyFence(device, inFlightFences[index], nullptr);
        }
    }
    if (commandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(device, commandPool, nullptr);
    }
}

void VulkanRenderer::createCommandPool() {
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = deviceCore.getGraphicsFamilyIndex();
    if (vkCreateCommandPool(
            deviceCore.getDevice(), &poolInfo, nullptr, &commandPool
        ) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create renderer command pool");
    }
}

void VulkanRenderer::createCommandBuffers() {
    VkCommandBufferAllocateInfo allocationInfo{};
    allocationInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocationInfo.commandPool = commandPool;
    allocationInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocationInfo.commandBufferCount = framesInFlight;
    if (vkAllocateCommandBuffers(
            deviceCore.getDevice(), &allocationInfo, commandBuffers.data()
        ) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate renderer command buffers");
    }
}

void VulkanRenderer::createSyncObjects() {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (uint32_t index = 0; index < framesInFlight; ++index) {
        if (vkCreateSemaphore(
                deviceCore.getDevice(), &semaphoreInfo, nullptr,
                &imageAvailableSemaphores[index]
            ) != VK_SUCCESS ||
            vkCreateSemaphore(
                deviceCore.getDevice(), &semaphoreInfo, nullptr,
                &renderFinishedSemaphores[index]
            ) != VK_SUCCESS ||
            vkCreateFence(
                deviceCore.getDevice(), &fenceInfo, nullptr,
                &inFlightFences[index]
            ) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create renderer synchronization objects");
        }
    }
}

VkCommandBuffer VulkanRenderer::beginFrame(uint32_t& imageIndex) {
    const VkDevice device = deviceCore.getDevice();
    if (vkWaitForFences(
            device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX
        ) != VK_SUCCESS) {
        throw std::runtime_error("Failed to wait for a frame fence");
    }

    const VkResult acquireResult = vkAcquireNextImageKHR(
        device,
        swapchainCore.get(),
        UINT64_MAX,
        imageAvailableSemaphores[currentFrame],
        VK_NULL_HANDLE,
        &imageIndex
    );
    if (acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Failed to acquire a swapchain image");
    }

    if (vkResetFences(device, 1, &inFlightFences[currentFrame]) != VK_SUCCESS ||
        vkResetCommandBuffer(commandBuffers[currentFrame], 0) != VK_SUCCESS) {
        throw std::runtime_error("Failed to reset frame synchronization state");
    }

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    if (vkBeginCommandBuffer(commandBuffers[currentFrame], &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin recording frame commands");
    }
    return commandBuffers[currentFrame];
}

void VulkanRenderer::endFrame(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to record frame commands");
    }

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    if (vkQueueSubmit(
            deviceCore.getGraphicsQueue(),
            1,
            &submitInfo,
            inFlightFences[currentFrame]
        ) != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit frame commands");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    const VkSwapchainKHR swapchains[] = {swapchainCore.get()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;
    const VkResult presentResult = vkQueuePresentKHR(
        deviceCore.getPresentQueue(), &presentInfo
    );
    if (presentResult != VK_SUCCESS && presentResult != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Failed to present the swapchain image");
    }

    currentFrame = (currentFrame + 1) % framesInFlight;
}
