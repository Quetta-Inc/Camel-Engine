#pragma once

#include "VulkanDevice.hpp"
#include "VulkanRenderPass.hpp"

#include <vulkan/vulkan.h>

#include <filesystem>
#include <string>
#include <vector>

class VulkanPipeline {
public:
    VulkanPipeline(
        const VulkanDevice& device,
        const VulkanRenderPass& renderPass,
        VkExtent2D extent,
        std::filesystem::path shaderDirectory
    );
    ~VulkanPipeline();

    VulkanPipeline(const VulkanPipeline&) = delete;
    VulkanPipeline& operator=(const VulkanPipeline&) = delete;

    VkPipeline get() const { return graphicsPipeline; }
    VkPipelineLayout getLayout() const { return pipelineLayout; }
    VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

private:
    void createDescriptorSetLayout();
    void createGraphicsPipeline(VkExtent2D extent, const std::filesystem::path& shaderDirectory);
    static std::vector<char> readFile(const std::filesystem::path& filename);
    VkShaderModule createShaderModule(const std::vector<char>& code) const;

    const VulkanDevice& deviceCore;
    const VulkanRenderPass& renderPassCore;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
};
