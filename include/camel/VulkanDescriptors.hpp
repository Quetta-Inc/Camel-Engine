#pragma once

#include "VulkanBuffer.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanTexture.hpp"

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

class VulkanDescriptors {
public:
    VulkanDescriptors(
        const VulkanDevice& device,
        const VulkanPipeline& pipeline,
        const std::vector<std::unique_ptr<VulkanBuffer>>& uniformBuffers,
        const VulkanTexture& texture
    );
    ~VulkanDescriptors();

    VulkanDescriptors(const VulkanDescriptors&) = delete;
    VulkanDescriptors& operator=(const VulkanDescriptors&) = delete;

    VkDescriptorSet get(size_t index) const { return descriptorSets.at(index); }

private:
    const VulkanDevice& deviceCore;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> descriptorSets;
};
