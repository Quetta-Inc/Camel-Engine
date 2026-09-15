#include "camel/VulkanDescriptors.hpp"

#include "camel/ApplicationConfig.hpp"
#include "camel/VulkanTypes.hpp"

#include <array>
#include <algorithm>
#include <stdexcept>

VulkanDescriptors::VulkanDescriptors(
    const VulkanDevice& device,
    const VulkanPipeline& pipeline,
    const std::vector<std::unique_ptr<VulkanBuffer>>& uniformBuffers,
    const VulkanTexture& texture
)
    : deviceCore(device) {
    if (uniformBuffers.size() != MAX_FRAMES_IN_FLIGHT) {
        throw std::invalid_argument("Uniform buffer count must match frames in flight");
    }

    const std::array<VkDescriptorPoolSize, 2> poolSizes = {{
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT}
    }};
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = MAX_FRAMES_IN_FLIGHT;
    if (vkCreateDescriptorPool(
            deviceCore.getDevice(), &poolInfo, nullptr, &descriptorPool
        ) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor pool");
    }

    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT);
    std::fill(layouts.begin(), layouts.end(), pipeline.getDescriptorSetLayout());
    VkDescriptorSetAllocateInfo allocationInfo{};
    allocationInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocationInfo.descriptorPool = descriptorPool;
    allocationInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
    allocationInfo.pSetLayouts = layouts.data();
    descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(
            deviceCore.getDevice(), &allocationInfo, descriptorSets.data()
        ) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate descriptor sets");
    }

    for (uint32_t index = 0; index < MAX_FRAMES_IN_FLIGHT; ++index) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[index]->getBuffer();
        bufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = texture.getImageView();
        imageInfo.sampler = texture.getSampler();

        const std::array<VkWriteDescriptorSet, 2> writes = {{
            {
                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr,
                descriptorSets[index], 0, 0, 1,
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nullptr, &bufferInfo, nullptr
            },
            {
                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr,
                descriptorSets[index], 1, 0, 1,
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &imageInfo, nullptr, nullptr
            }
        }};
        vkUpdateDescriptorSets(
            deviceCore.getDevice(),
            static_cast<uint32_t>(writes.size()),
            writes.data(),
            0,
            nullptr
        );
    }
}

VulkanDescriptors::~VulkanDescriptors() {
    if (descriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(deviceCore.getDevice(), descriptorPool, nullptr);
    }
}
