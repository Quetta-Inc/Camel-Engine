#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "VulkanDevice.hpp"
#include "VulkanSwapchain.hpp"

class VulkanRenderer {
public:
    // Pass the swapchain into the constructor
    VulkanRenderer(const VulkanDevice& device, const VulkanSwapchain& swapchain);
    ~VulkanRenderer();

    VulkanRenderer(const VulkanRenderer&) = delete;
    VulkanRenderer& operator=(const VulkanRenderer&) = delete;

    // Output the acquired image index
    VkCommandBuffer beginFrame(uint32_t& outImageIndex);
    
    // Take the image index to present it
    void endFrame(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    
    VkCommandPool getCommandPool() const { return commandPool; }
    uint32_t getCurrentFrame() const { return currentFrame; }
    
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

private:
    void createCommandPool();
    void createCommandBuffers();
    void createSyncObjects();

    const VulkanDevice& deviceCore;
    const VulkanSwapchain& swapchainCore; // <-- Add this reference

    VkCommandPool commandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame = 0;
};
