#pragma once

#include "VulkanDepthBuffer.hpp"
#include "VulkanDevice.hpp"
#include "VulkanImageViews.hpp"
#include "VulkanRenderPass.hpp"

#include <vulkan/vulkan.h>

#include <vector>

class VulkanFramebuffers {
public:
    VulkanFramebuffers(
        const VulkanDevice& device,
        const VulkanSwapchain& swapchain,
        const VulkanImageViews& imageViews,
        const VulkanDepthBuffer& depthBuffer,
        const VulkanRenderPass& renderPass
    );
    ~VulkanFramebuffers();

    VulkanFramebuffers(const VulkanFramebuffers&) = delete;
    VulkanFramebuffers& operator=(const VulkanFramebuffers&) = delete;

    VkFramebuffer get(size_t index) const { return framebuffers.at(index); }
    size_t size() const { return framebuffers.size(); }

private:
    const VulkanDevice& deviceCore;
    std::vector<VkFramebuffer> framebuffers;
};
