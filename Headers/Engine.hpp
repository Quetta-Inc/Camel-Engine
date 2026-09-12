#pragma once

#include <SDL3/SDL.h>
#include <memory>
#include <vector>

#include "initialization.hpp"
#include "VulkanDevice.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanRenderer.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanTexture.hpp"
#include "Primitive.hpp"
#include "Camera.hpp"

struct RenderObject {
    std::unique_ptr<VulkanBuffer> vertexBuffer;
    std::unique_ptr<VulkanBuffer> indexBuffer;
    uint32_t indexCount;
    glm::vec3 position;
    
    glm::mat4 getModelMatrix() const {
        return glm::translate(glm::mat4(1.0f), position);
    }
};

class Engine {
public:
    Engine();
    ~Engine();

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    void run();
    void addPrimitive(const MeshData& mesh, glm::vec3 position);

private:
    void initWindow();
    void initGraphics();
    void createUniformBuffers();
    void createDescriptorPoolAndSets();
    void updateUniformBuffer(uint32_t currentImage);
    void recordDrawCommands(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    void createDepthResources();
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat findDepthFormat();
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    SDL_Window* window = nullptr;

    std::unique_ptr<VulkanDevice> device;
    std::unique_ptr<VulkanSwapchain> swapchain;
    std::unique_ptr<VulkanPipeline> pipeline;
    std::unique_ptr<VulkanRenderer> renderer;
    std::unique_ptr<VulkanTexture> texture;

    std::vector<RenderObject> sceneObjects;
    std::vector<std::unique_ptr<VulkanBuffer>> uniformBuffers;

    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<VkFramebuffer> framebuffers;

    VkImage depthImage = VK_NULL_HANDLE;
    VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;
    VkImageView depthImageView = VK_NULL_HANDLE;

    Camera camera;
    float lastFrameTime = 0.0f;
};
