#pragma once

#include "VulkanDepthBuffer.hpp"
#include "VulkanDevice.hpp"
#include "VulkanSwapchain.hpp"

#include <vulkan/vulkan.h>

class VulkanRenderPass {
public:
    VulkanRenderPass(
        const VulkanDevice& device,
        const VulkanSwapchain& swapchain,
        const VulkanDepthBuffer& depthBuffer
    );
    ~VulkanRenderPass();

    VulkanRenderPass(const VulkanRenderPass&) = delete;
    VulkanRenderPass& operator=(const VulkanRenderPass&) = delete;

    VkRenderPass get() const { return renderPass; }

private:
    const VulkanDevice& deviceCore;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};
