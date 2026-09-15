#include "camel/VulkanFramebuffers.hpp"

#include "camel/Logger.hpp"

#include <array>
#include <stdexcept>

VulkanFramebuffers::VulkanFramebuffers(
    const VulkanDevice& device,
    const VulkanSwapchain& swapchain,
    const VulkanImageViews& imageViews,
    const VulkanDepthBuffer& depthBuffer,
    const VulkanRenderPass& renderPass
)
    : deviceCore(device) {
    if (imageViews.size() != swapchain.getImageCount()) {
        throw std::invalid_argument("There must be one framebuffer per swapchain image");
    }

    framebuffers.reserve(imageViews.size());
    try {
        for (size_t index = 0; index < imageViews.size(); ++index) {
            const std::array<VkImageView, 2> attachments = {
                imageViews.get().at(index),
                depthBuffer.getImageView()
            };

            VkFramebufferCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            createInfo.renderPass = renderPass.get();
            createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            createInfo.pAttachments = attachments.data();
            createInfo.width = swapchain.getExtent().width;
            createInfo.height = swapchain.getExtent().height;
            createInfo.layers = 1;

            VkFramebuffer framebuffer = VK_NULL_HANDLE;
            if (vkCreateFramebuffer(
                    deviceCore.getDevice(), &createInfo, nullptr, &framebuffer
                ) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create framebuffer");
            }
            framebuffers.push_back(framebuffer);
        }
    } catch (...) {
        for (VkFramebuffer framebuffer : framebuffers) {
            vkDestroyFramebuffer(deviceCore.getDevice(), framebuffer, nullptr);
        }
        throw;
    }

    Logger::log(Logger::LogLevel::Info, "Framebuffers created.");
}

VulkanFramebuffers::~VulkanFramebuffers() {
    for (VkFramebuffer framebuffer : framebuffers) {
        if (framebuffer != VK_NULL_HANDLE) {
            vkDestroyFramebuffer(deviceCore.getDevice(), framebuffer, nullptr);
        }
    }
}
