#define SDL_MAIN_HANDLED

#include "camel/ApplicationConfig.hpp"
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
#include "camel/VulkanTypes.hpp"

#include <SDL3/SDL.h>

#include <filesystem>
#include <iostream>
#include <memory>
#include <vector>

#ifndef CAMEL_ENGINE_TEST_SHADER_DIR
#define CAMEL_ENGINE_TEST_SHADER_DIR "."
#endif

#ifndef CAMEL_ENGINE_TEST_ASSET_DIR
#define CAMEL_ENGINE_TEST_ASSET_DIR "."
#endif

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << '\n';
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Camel Engine component test",
        static_cast<int>(WINDOW_WIDTH),
        static_cast<int>(WINDOW_HEIGHT),
        SDL_WINDOW_VULKAN
    );
    if (window == nullptr) {
        std::cerr << "SDL window creation failed: " << SDL_GetError() << '\n';
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
            std::filesystem::path(CAMEL_ENGINE_TEST_SHADER_DIR)
        );
        VulkanRenderer renderer(device, swapchain);
        VulkanTexture texture(
            device,
            (std::filesystem::path(CAMEL_ENGINE_TEST_ASSET_DIR) / "texture.png").string(),
            commandExecutor
        );

        std::vector<std::unique_ptr<VulkanBuffer>> uniformBuffers;
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

        if (imageViews.size() != swapchain.getImageCount() ||
            framebuffers.size() != swapchain.getImageCount()) {
            std::cerr << "Swapchain dependent component counts do not match\n";
            result = 1;
        }
    } catch (const std::exception& exception) {
        std::cerr << "Vulkan component test failed: " << exception.what() << '\n';
        result = 1;
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return result;
}
