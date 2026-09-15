#define SDL_MAIN_HANDLED

#include "camel/ApplicationConfig.hpp"
#include "camel/Logger.hpp"
#include "camel/Primitive.hpp"
#include "camel/VulkanBuffer.hpp"
#include "camel/VulkanCommandExecutor.hpp"
#include "camel/VulkanDepthBuffer.hpp"
#include "camel/VulkanDescriptors.hpp"
#include "camel/VulkanDevice.hpp"
#include "camel/VulkanFramebuffers.hpp"
#include "camel/VulkanImageViews.hpp"
#include "camel/VulkanInstance.hpp"
#include "camel/VulkanPipeline.hpp"
#include "camel/VulkanPhysicalDevice.hpp"
#include "camel/VulkanRenderPass.hpp"
#include "camel/VulkanRenderer.hpp"
#include "camel/VulkanSurface.hpp"
#include "camel/VulkanSwapchain.hpp"
#include "camel/VulkanTexture.hpp"

#include <SDL3/SDL.h>
#include <glm/gtc/matrix_transform.hpp>

#include <array>
#include <exception>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace {

void updateUniformBuffer(
    VulkanBuffer& uniformBuffer,
    const VulkanSwapchain& swapchain
) {
    UniformBufferObject uniform{};
    uniform.view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 3.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    uniform.proj = glm::perspective(
        glm::radians(45.0f),
        static_cast<float>(swapchain.getExtent().width) /
            static_cast<float>(swapchain.getExtent().height),
        0.1f,
        100.0f
    );
    uniform.proj[1][1] *= -1.0f;
    uniformBuffer.mapMemory(&uniform, sizeof(uniform));
}

void recordCube(
    VkCommandBuffer commandBuffer,
    uint32_t imageIndex,
    const VulkanSwapchain& swapchain,
    const VulkanRenderPass& renderPass,
    const VulkanFramebuffers& framebuffers,
    const VulkanPipeline& pipeline,
    const VulkanRenderer& renderer,
    const VulkanDescriptors& descriptors,
    const MeshData& mesh,
    const VulkanBuffer& vertexBuffer,
    const VulkanBuffer& indexBuffer
) {
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass.get();
    renderPassInfo.framebuffer = framebuffers.get(imageIndex);
    renderPassInfo.renderArea.extent = swapchain.getExtent();

    const std::array<VkClearValue, 2> clearValues = {{
        {{{0.02f, 0.02f, 0.04f, 1.0f}}},
        {{{1.0f, 0}}}
    }};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(
        commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE
    );
    vkCmdBindPipeline(
        commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.get()
    );

    const VkBuffer vertexBuffers[] = {vertexBuffer.getBuffer()};
    const VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(
        commandBuffer, indexBuffer.getBuffer(), 0, VK_INDEX_TYPE_UINT16
    );

    const VkDescriptorSet descriptorSet =
        descriptors.get(renderer.getCurrentFrame());
    vkCmdBindDescriptorSets(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipeline.getLayout(),
        0,
        1,
        &descriptorSet,
        0,
        nullptr
    );

    const glm::mat4 model = glm::mat4(1.0f);
    vkCmdPushConstants(
        commandBuffer,
        pipeline.getLayout(),
        VK_SHADER_STAGE_VERTEX_BIT,
        0,
        sizeof(model),
        &model
    );
    vkCmdDrawIndexed(
        commandBuffer,
        static_cast<uint32_t>(mesh.indices.size()),
        1,
        0,
        0,
        0
    );
    vkCmdEndRenderPass(commandBuffer);
}

} // namespace

int main(int argc, char* argv[]) {
    const bool smokeTest = argc > 1 && std::string(argv[1]) == "--smoke-test";
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        Logger::log(Logger::LogLevel::Error, SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        APP_NAME,
        static_cast<int>(WINDOW_WIDTH),
        static_cast<int>(WINDOW_HEIGHT),
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
    );
    if (window == nullptr) {
        Logger::log(Logger::LogLevel::Error, SDL_GetError());
        SDL_Quit();
        return 1;
    }

    int result = 0;
    try {
        VulkanInstance instance;
        VulkanSurface surface(instance, window);
        VulkanPhysicalDevice physicalDevice(instance, surface);
        VulkanDevice device(physicalDevice);
        VulkanCommandExecutor commandExecutor(device);
        VulkanSwapchain swapchain(device, surface);
        VulkanImageViews imageViews(device, swapchain);
        VulkanDepthBuffer depthBuffer(device, swapchain);
        VulkanRenderPass renderPass(device, swapchain, depthBuffer);
        VulkanPipeline pipeline(
            device,
            renderPass,
            swapchain.getExtent(),
            std::filesystem::path(CAMEL_ENGINE_SHADER_DIR)
        );
        VulkanRenderer renderer(device, swapchain);
        VulkanTexture texture(
            device,
            (std::filesystem::path(CAMEL_ENGINE_ASSET_DIR) / "texture.png").string(),
            commandExecutor
        );

        std::vector<std::unique_ptr<VulkanBuffer>> uniformBuffers;
        uniformBuffers.reserve(MAX_FRAMES_IN_FLIGHT);
        for (uint32_t index = 0; index < MAX_FRAMES_IN_FLIGHT; ++index) {
            uniformBuffers.push_back(std::make_unique<VulkanBuffer>(
                device,
                sizeof(UniformBufferObject),
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            ));
        }
        VulkanDescriptors descriptors(device, pipeline, uniformBuffers, texture);
        VulkanFramebuffers framebuffers(
            device, swapchain, imageViews, depthBuffer, renderPass
        );

        const MeshData cube = Primitive::createCube();
        const VkDeviceSize vertexSize = sizeof(Vertex) * cube.vertices.size();
        VulkanBuffer stagingVertices(
            device,
            vertexSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );
        stagingVertices.mapMemory(cube.vertices.data(), vertexSize);
        VulkanBuffer vertexBuffer(
            device,
            vertexSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );
        commandExecutor.copyBuffer(
            stagingVertices.getBuffer(), vertexBuffer.getBuffer(), vertexSize
        );

        const VkDeviceSize indexSize = sizeof(uint16_t) * cube.indices.size();
        VulkanBuffer stagingIndices(
            device,
            indexSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );
        stagingIndices.mapMemory(cube.indices.data(), indexSize);
        VulkanBuffer indexBuffer(
            device,
            indexSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );
        commandExecutor.copyBuffer(
            stagingIndices.getBuffer(), indexBuffer.getBuffer(), indexSize
        );

        bool running = true;
        while (running) {
            SDL_Event event{};
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_QUIT) {
                    running = false;
                }
            }

            uint32_t imageIndex = 0;
            const VkCommandBuffer commandBuffer = renderer.beginFrame(imageIndex);
            updateUniformBuffer(
                *uniformBuffers[renderer.getCurrentFrame()], swapchain
            );
            recordCube(
                commandBuffer,
                imageIndex,
                swapchain,
                renderPass,
                framebuffers,
                pipeline,
                renderer,
                descriptors,
                cube,
                vertexBuffer,
                indexBuffer
            );
            renderer.endFrame(commandBuffer, imageIndex);
            if (smokeTest) {
                running = false;
            }
        }
        vkDeviceWaitIdle(device.getDevice());
    } catch (const std::exception& exception) {
        Logger::log(Logger::LogLevel::Error, exception.what());
        result = 1;
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return result;
}
