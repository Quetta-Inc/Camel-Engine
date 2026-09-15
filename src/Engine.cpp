#include "camel/Engine.hpp"

#include "camel/Logger.hpp"

#include <SDL3/SDL.h>

#include <array>
#include <filesystem>
#include <stdexcept>
#include <string>

#ifndef CAMEL_ENGINE_SHADER_DIR
#define CAMEL_ENGINE_SHADER_DIR "."
#endif

#ifndef CAMEL_ENGINE_ASSET_DIR
#define CAMEL_ENGINE_ASSET_DIR "."
#endif

Engine::Engine() {
    try {
        initWindow();
        initGraphics();
    } catch (...) {
        destroyGraphics();
        if (window != nullptr) {
            SDL_DestroyWindow(window);
            window = nullptr;
        }
        SDL_Quit();
        throw;
    }
    Logger::log(Logger::LogLevel::Info, "Engine fully initialized.");
}

Engine::~Engine() {
    destroyGraphics();
    if (window != nullptr) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    SDL_Quit();
    Logger::log(Logger::LogLevel::Info, "Engine shut down cleanly.");
}

void Engine::initWindow() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        throw std::runtime_error(
            std::string("Failed to initialize SDL: ") + SDL_GetError()
        );
    }

    window = SDL_CreateWindow(
        APP_NAME,
        static_cast<int>(WINDOW_WIDTH),
        static_cast<int>(WINDOW_HEIGHT),
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
    );
    if (window == nullptr) {
        throw std::runtime_error(
            std::string("Failed to create SDL window: ") + SDL_GetError()
        );
    }

    if (!SDL_SetWindowRelativeMouseMode(window, true)) {
        Logger::log(Logger::LogLevel::Warning, "Relative mouse mode is unavailable.");
        isMouseLocked = false;
    }
}

void Engine::initGraphics() {
    instance = std::make_unique<VulkanInstance>();
    surface = std::make_unique<VulkanSurface>(*instance, window);
    physicalDevice = std::make_unique<VulkanPhysicalDevice>(*instance, *surface);
    device = std::make_unique<VulkanDevice>(*physicalDevice);
    commandExecutor = std::make_unique<VulkanCommandExecutor>(*device);
    swapchain = std::make_unique<VulkanSwapchain>(*device, *surface);
    imageViews = std::make_unique<VulkanImageViews>(*device, *swapchain);
    depthBuffer = std::make_unique<VulkanDepthBuffer>(*device, *swapchain);
    renderPass = std::make_unique<VulkanRenderPass>(*device, *swapchain, *depthBuffer);
    pipeline = std::make_unique<VulkanPipeline>(
        *device,
        *renderPass,
        swapchain->getExtent(),
        std::filesystem::path(CAMEL_ENGINE_SHADER_DIR)
    );
    renderer = std::make_unique<VulkanRenderer>(*device, *swapchain);
    texture = std::make_unique<VulkanTexture>(
        *device,
        (std::filesystem::path(CAMEL_ENGINE_ASSET_DIR) / "texture.png").string(),
        *commandExecutor
    );

    createUniformBuffers();
    descriptors = std::make_unique<VulkanDescriptors>(
        *device, *pipeline, uniformBuffers, *texture
    );
    framebuffers = std::make_unique<VulkanFramebuffers>(
        *device, *swapchain, *imageViews, *depthBuffer, *renderPass
    );
}

void Engine::destroyGraphics() {
    if (device != nullptr) {
        vkDeviceWaitIdle(device->getDevice());
    }

    framebuffers.reset();
    descriptors.reset();
    sceneObjects.clear();
    uniformBuffers.clear();
    texture.reset();
    renderer.reset();
    pipeline.reset();
    renderPass.reset();
    depthBuffer.reset();
    imageViews.reset();
    swapchain.reset();
    commandExecutor.reset();
    device.reset();
    physicalDevice.reset();
    surface.reset();
    instance.reset();
}

void Engine::createUniformBuffers() {
    uniformBuffers.reserve(MAX_FRAMES_IN_FLIGHT);
    for (uint32_t index = 0; index < MAX_FRAMES_IN_FLIGHT; ++index) {
        uniformBuffers.push_back(std::make_unique<VulkanBuffer>(
            *device,
            sizeof(UniformBufferObject),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        ));
    }
}

RenderObject* Engine::addPrimitive(const MeshData& mesh, glm::vec3 position) {
    if (mesh.vertices.empty() || mesh.indices.empty()) {
        throw std::invalid_argument("A renderable primitive must contain vertices and indices");
    }
    if (device == nullptr || commandExecutor == nullptr) {
        throw std::logic_error("Cannot add a primitive before graphics initialization");
    }

    auto object = std::make_unique<RenderObject>();
    object->transform.position = position;
    object->indexCount = static_cast<uint32_t>(mesh.indices.size());

    const VkDeviceSize vertexSize =
        sizeof(mesh.vertices.front()) * mesh.vertices.size();
    VulkanBuffer stagingVertices(
        *device,
        vertexSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );
    stagingVertices.mapMemory(mesh.vertices.data(), vertexSize);
    object->vertexBuffer = std::make_unique<VulkanBuffer>(
        *device,
        vertexSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    commandExecutor->copyBuffer(
        stagingVertices.getBuffer(), object->vertexBuffer->getBuffer(), vertexSize
    );

    const VkDeviceSize indexSize =
        sizeof(mesh.indices.front()) * mesh.indices.size();
    VulkanBuffer stagingIndices(
        *device,
        indexSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );
    stagingIndices.mapMemory(mesh.indices.data(), indexSize);
    object->indexBuffer = std::make_unique<VulkanBuffer>(
        *device,
        indexSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    commandExecutor->copyBuffer(
        stagingIndices.getBuffer(), object->indexBuffer->getBuffer(), indexSize
    );

    RenderObject* result = object.get();
    sceneObjects.push_back(std::move(object));
    return result;
}

void Engine::updateUniformBuffer(uint32_t currentImage) {
    if (currentImage >= uniformBuffers.size()) {
        throw std::out_of_range("Uniform buffer frame index is out of range");
    }

    UniformBufferObject uniform{};
    uniform.view = camera.getViewMatrix();
    uniform.proj = glm::perspective(
        glm::radians(45.0f),
        static_cast<float>(swapchain->getExtent().width) /
            static_cast<float>(swapchain->getExtent().height),
        0.1f,
        100.0f
    );
    uniform.proj[1][1] *= -1.0f;
    uniformBuffers[currentImage]->mapMemory(&uniform, sizeof(uniform));
}

void Engine::recordDrawCommands(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    if (imageIndex >= framebuffers->size()) {
        throw std::out_of_range("Swapchain image index is out of range");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass->get();
    renderPassInfo.framebuffer = framebuffers->get(imageIndex);
    renderPassInfo.renderArea.extent = swapchain->getExtent();

    const std::array<VkClearValue, 2> clearValues = {{
        {{{0.03f, 0.06f, 0.12f, 1.0f}}},
        {{{1.0f, 0}}}
    }};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(
        commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE
    );
    vkCmdBindPipeline(
        commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get()
    );

    for (const auto& object : sceneObjects) {
        const VkBuffer vertexBuffers[] = {object->vertexBuffer->getBuffer()};
        const VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(
            commandBuffer, object->indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT16
        );
        const VkDescriptorSet descriptorSet =
            descriptors->get(renderer->getCurrentFrame());
        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeline->getLayout(),
            0,
            1,
            &descriptorSet,
            0,
            nullptr
        );

        const glm::mat4 model = object->transform.getModelMatrix();
        vkCmdPushConstants(
            commandBuffer,
            pipeline->getLayout(),
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof(model),
            &model
        );
        vkCmdDrawIndexed(commandBuffer, object->indexCount, 1, 0, 0, 0);
    }

    vkCmdEndRenderPass(commandBuffer);
}

bool Engine::isKeyPressed(SDL_Scancode key) const {
    if (key < 0 || key >= SDL_SCANCODE_COUNT) {
        return false;
    }
    const bool* state = SDL_GetKeyboardState(nullptr);
    return state != nullptr && state[key];
}

void Engine::setMouseLock(bool locked) {
    isMouseLocked = locked;
    if (window != nullptr && !SDL_SetWindowRelativeMouseMode(window, locked)) {
        Logger::log(Logger::LogLevel::Warning, "Could not change relative mouse mode.");
    }
}

void Engine::run(bool smokeTest) {
    bool running = true;
    SDL_Event event{};
    start();
    lastFrameTime = static_cast<float>(SDL_GetTicks()) / 1000.0f;

    while (running) {
        const float currentFrameTime =
            static_cast<float>(SDL_GetTicks()) / 1000.0f;
        const float deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            } else if (event.type == SDL_EVENT_MOUSE_MOTION && isMouseLocked) {
                camera.processMouseMovement(event.motion.xrel, event.motion.yrel);
            } else if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
                onKeyDown(event.key.scancode);
            }
        }

        if (isMouseLocked) {
            camera.processKeyboard(SDL_GetKeyboardState(nullptr), deltaTime);
        }
        update(deltaTime);

        uint32_t imageIndex = 0;
        const VkCommandBuffer commandBuffer = renderer->beginFrame(imageIndex);
        updateUniformBuffer(renderer->getCurrentFrame());
        recordDrawCommands(commandBuffer, imageIndex);
        renderer->endFrame(commandBuffer, imageIndex);

        if (smokeTest) {
            running = false;
        }
    }
}
