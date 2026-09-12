#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include "VulkanDevice.hpp"
#include "VulkanSwapchain.hpp"

class VulkanPipeline {
public:
    VulkanPipeline(const VulkanDevice& device, const VulkanSwapchain& swapchain);
    ~VulkanPipeline();

    // Prevent copying
    VulkanPipeline(const VulkanPipeline&) = delete;
    VulkanPipeline& operator=(const VulkanPipeline&) = delete;

    VkPipeline getPipeline() const { return graphicsPipeline; }
    VkPipelineLayout getLayout() const { return pipelineLayout; }
    VkRenderPass getRenderPass() const { return renderPass; }
    VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

private:
    void createRenderPass(VkFormat swapchainImageFormat, VkFormat depthFormat);
    void createDescriptorSetLayout();
    void createGraphicsPipeline(VkExtent2D swapchainExtent);
    static std::vector<char> readFile(const std::string& filename);
    VkShaderModule createShaderModule(const std::vector<char>& code);

    const VulkanDevice& deviceCore;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
};
