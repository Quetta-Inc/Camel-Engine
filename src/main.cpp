#define SDL_MAIN_HANDLED

#include "camel/ApplicationConfig.hpp"
#include "camel/Logger.hpp"
#include "camel/VulkanDepthBuffer.hpp"
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

#include <SDL3/SDL.h>

#include <array>
#include <exception>
#include <filesystem>
#include <string>

namespace {

void recordTriangle(
    VkCommandBuffer commandBuffer,
    uint32_t imageIndex,
    const VulkanRenderer& renderer,
    const VulkanSwapchain& swapchain,
    const VulkanRenderPass& renderPass,
    const VulkanFramebuffers& framebuffers,
    const VulkanPipeline& pipeline
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
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
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
        VulkanFramebuffers framebuffers(
            device, swapchain, imageViews, depthBuffer, renderPass
        );
        VulkanRenderer renderer(device, swapchain);

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
            recordTriangle(
                commandBuffer,
                imageIndex,
                renderer,
                swapchain,
                renderPass,
                framebuffers,
                pipeline
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
