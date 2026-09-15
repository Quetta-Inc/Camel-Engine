#pragma once

#include "ApplicationConfig.hpp"
#include "VulkanDevice.hpp"
#include "VulkanSwapchain.hpp"

#include <vulkan/vulkan.h>

#include <array>

class VulkanRenderer {
public:
    VulkanRenderer(const VulkanDevice& device, const VulkanSwapchain& swapchain);
    ~VulkanRenderer();

    VulkanRenderer(const VulkanRenderer&) = delete;
    VulkanRenderer& operator=(const VulkanRenderer&) = delete;

    VkCommandBuffer beginFrame(uint32_t& imageIndex);
    void endFrame(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    VkCommandPool getCommandPool() const { return commandPool; }
    uint32_t getCurrentFrame() const { return currentFrame; }
    static constexpr uint32_t framesInFlight = MAX_FRAMES_IN_FLIGHT;

private:
    void createCommandPool();
    void createCommandBuffers();
    void createSyncObjects();

    const VulkanDevice& deviceCore;
    const VulkanSwapchain& swapchainCore;
    VkCommandPool commandPool = VK_NULL_HANDLE;
    std::array<VkCommandBuffer, framesInFlight> commandBuffers{};
    std::array<VkSemaphore, framesInFlight> imageAvailableSemaphores{};
    std::array<VkSemaphore, framesInFlight> renderFinishedSemaphores{};
    std::array<VkFence, framesInFlight> inFlightFences{};
    uint32_t currentFrame = 0;
};
