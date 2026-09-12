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

struct Transform {
    glm::vec3 position{0.0f};
    glm::vec3 rotation{0.0f}; // In degrees
    glm::vec3 scale{1.0f};

    glm::mat4 getModelMatrix() const {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
        model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, scale);
        return model;
    }
};

struct RenderObject {
    std::unique_ptr<VulkanBuffer> vertexBuffer;
    std::unique_ptr<VulkanBuffer> indexBuffer;
    uint32_t indexCount;
    Transform transform;
};

class Engine {
public:
    Engine();
    virtual ~Engine();

    RenderObject* addPrimitive(const MeshData& mesh, glm::vec3 startPosition = glm::vec3(0.0f));

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    void run();
    bool isKeyPressed(SDL_Scancode key) const;
    void setMouseLock(bool locked);
    bool getMouseLock() const { return isMouseLocked; }

protected:

    virtual void start() {}
    virtual void update(float deltaTime) {}

    virtual void onKeyDown(SDL_Scancode key) {}

    Camera camera;

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

    bool isMouseLocked = true;

    SDL_Window* window = nullptr;

    std::unique_ptr<VulkanDevice> device;
    std::unique_ptr<VulkanSwapchain> swapchain;
    std::unique_ptr<VulkanPipeline> pipeline;
    std::unique_ptr<VulkanRenderer> renderer;
    std::unique_ptr<VulkanTexture> texture;

    // Use unique_ptr to keep memory addresses stable when returning pointers
    std::vector<std::unique_ptr<RenderObject>> sceneObjects; 
    std::vector<std::unique_ptr<VulkanBuffer>> uniformBuffers;

    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<VkFramebuffer> framebuffers;

    VkImage depthImage = VK_NULL_HANDLE;
    VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;
    VkImageView depthImageView = VK_NULL_HANDLE;

    float lastFrameTime = 0.0f;
};
