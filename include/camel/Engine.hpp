#pragma once

#include "ApplicationConfig.hpp"
#include "Camera.hpp"
#include "Primitive.hpp"
#include "Scene.hpp"
#include "VulkanCommandExecutor.hpp"
#include "VulkanDepthBuffer.hpp"
#include "VulkanDescriptors.hpp"
#include "VulkanDevice.hpp"
#include "VulkanFramebuffers.hpp"
#include "VulkanImageViews.hpp"
#include "VulkanInstance.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanPhysicalDevice.hpp"
#include "VulkanRenderPass.hpp"
#include "VulkanRenderer.hpp"
#include "VulkanSurface.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanTexture.hpp"

#include <SDL3/SDL.h>
#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

class Engine {
public:
    Engine();
    virtual ~Engine();

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    RenderObject* addPrimitive(
        const MeshData& mesh,
        glm::vec3 startPosition = glm::vec3(0.0f)
    );
    void run(bool smokeTest = false);
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
    void destroyGraphics();
    void createUniformBuffers();
    void updateUniformBuffer(uint32_t currentImage);
    void recordDrawCommands(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    bool isMouseLocked = true;
    SDL_Window* window = nullptr;

    std::unique_ptr<VulkanInstance> instance;
    std::unique_ptr<VulkanSurface> surface;
    std::unique_ptr<VulkanPhysicalDevice> physicalDevice;
    std::unique_ptr<VulkanDevice> device;
    std::unique_ptr<VulkanCommandExecutor> commandExecutor;
    std::unique_ptr<VulkanSwapchain> swapchain;
    std::unique_ptr<VulkanImageViews> imageViews;
    std::unique_ptr<VulkanDepthBuffer> depthBuffer;
    std::unique_ptr<VulkanRenderPass> renderPass;
    std::unique_ptr<VulkanPipeline> pipeline;
    std::unique_ptr<VulkanRenderer> renderer;
    std::unique_ptr<VulkanTexture> texture;
    std::vector<std::unique_ptr<VulkanBuffer>> uniformBuffers;
    std::unique_ptr<VulkanDescriptors> descriptors;
    std::unique_ptr<VulkanFramebuffers> framebuffers;
    std::vector<std::unique_ptr<RenderObject>> sceneObjects;
    float lastFrameTime = 0.0f;
};
