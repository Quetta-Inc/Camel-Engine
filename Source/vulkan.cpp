#include "Headers/vulkan.hpp"
#include "Headers/initialization.hpp"
#include <SDL3/SDL_vulkan.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <string>
#include <cstring>
#include <chrono>

// Геометрия 3D Куба
const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}}, // 0
    {{ 0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}}, // 1
    {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}}, // 2
    {{-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}}, // 3
    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}}, // 4
    {{ 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}}, // 5
    {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}}, // 6
    {{-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}}  // 7
};

const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0, // front
    1, 5, 6, 6, 2, 1, // right
    5, 4, 7, 7, 6, 5, // back
    4, 0, 3, 3, 7, 4, // left
    3, 2, 6, 6, 7, 3, // top
    4, 5, 1, 1, 0, 4  // bottom
};

static std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) throw std::runtime_error("Failed to open file: " + filename);
    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
}

static VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    VkShaderModule shaderModule;
    vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);
    return shaderModule;
}

uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    throw std::runtime_error("Failed to find suitable memory type!");
}

void createBuffer(VulkanContext& ctx, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkCreateBuffer(ctx.device, &bufferInfo, nullptr, &buffer);

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(ctx.device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(ctx.physicalDevice, memRequirements.memoryTypeBits, properties);
    vkAllocateMemory(ctx.device, &allocInfo, nullptr, &bufferMemory);
    vkBindBufferMemory(ctx.device, buffer, bufferMemory, 0);
}

void copyBuffer(VulkanContext& ctx, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = ctx.commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(ctx.device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(ctx.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(ctx.graphicsQueue);
    vkFreeCommandBuffers(ctx.device, ctx.commandPool, 1, &commandBuffer);
}

void createImage(VulkanContext& ctx, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkCreateImage(ctx.device, &imageInfo, nullptr, &image);

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(ctx.device, image, &memRequirements);
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(ctx.physicalDevice, memRequirements.memoryTypeBits, properties);
    vkAllocateMemory(ctx.device, &allocInfo, nullptr, &imageMemory);
    vkBindImageMemory(ctx.device, image, imageMemory, 0);
}

VkImageView createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    VkImageView imageView;
    vkCreateImageView(device, &viewInfo, nullptr, &imageView);
    return imageView;
}

VkFormat findSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) return format;
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) return format;
    }
    throw std::runtime_error("Failed to find supported format!");
}

VkFormat findDepthFormat(VkPhysicalDevice physicalDevice) {
    return findSupportedFormat(physicalDevice,
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

void updateUniformBuffer(VulkanContext& ctx, uint32_t currentImage) {
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.model = glm::rotate(ubo.model, time * glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), ctx.swapchainExtent.width / (float)ctx.swapchainExtent.height, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1; // Исправление Y-координаты для Vulkan

    memcpy(ctx.uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

bool initVulkan(SDL_Window* window, VulkanContext& ctx) {
    try {
        uint32_t extensionCount = 0;
        char const* const* sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
        
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = APP_NAME;
        appInfo.apiVersion = VK_API_VERSION_1_1;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = extensionCount;
        createInfo.ppEnabledExtensionNames = sdlExtensions;

        if (vkCreateInstance(&createInfo, nullptr, &ctx.instance) != VK_SUCCESS) return false;
        if (!SDL_Vulkan_CreateSurface(window, ctx.instance, nullptr, &ctx.surface)) return false;

        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(ctx.instance, &deviceCount, nullptr);
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(ctx.instance, &deviceCount, devices.data());
        ctx.physicalDevice = devices[0];

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(ctx.physicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(ctx.physicalDevice, &queueFamilyCount, queueFamilies.data());

        for (uint32_t i = 0; i < queueFamilyCount; i++) {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) ctx.graphicsFamilyIndex = i;
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(ctx.physicalDevice, i, ctx.surface, &presentSupport);
            if (presentSupport) ctx.presentFamilyIndex = i;
        }

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {ctx.graphicsFamilyIndex, ctx.presentFamilyIndex};
        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
        deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

        if (vkCreateDevice(ctx.physicalDevice, &deviceCreateInfo, nullptr, &ctx.device) != VK_SUCCESS) return false;
        vkGetDeviceQueue(ctx.device, ctx.graphicsFamilyIndex, 0, &ctx.graphicsQueue);
        vkGetDeviceQueue(ctx.device, ctx.presentFamilyIndex, 0, &ctx.presentQueue);

        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ctx.physicalDevice, ctx.surface, &capabilities);
        ctx.swapchainExtent = capabilities.currentExtent;
        ctx.swapchainImageFormat = VK_FORMAT_B8G8R8A8_SRGB;

        VkSwapchainCreateInfoKHR swapchainInfo{};
        swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainInfo.surface = ctx.surface;
        swapchainInfo.minImageCount = capabilities.minImageCount + 1;
        swapchainInfo.imageFormat = ctx.swapchainImageFormat;
        swapchainInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        swapchainInfo.imageExtent = ctx.swapchainExtent;
        swapchainInfo.imageArrayLayers = 1;
        swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainInfo.preTransform = capabilities.currentTransform;
        swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchainInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
        swapchainInfo.clipped = VK_TRUE;

        vkCreateSwapchainKHR(ctx.device, &swapchainInfo, nullptr, &ctx.swapchain);

        uint32_t imageCount;
        vkGetSwapchainImagesKHR(ctx.device, ctx.swapchain, &imageCount, nullptr);
        ctx.swapchainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(ctx.device, ctx.swapchain, &imageCount, ctx.swapchainImages.data());

        ctx.swapchainImageViews.resize(ctx.swapchainImages.size());
        for (size_t i = 0; i < ctx.swapchainImages.size(); i++) {
            ctx.swapchainImageViews[i] = createImageView(ctx.device, ctx.swapchainImages[i], ctx.swapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
        }

        // --- Render Pass с буфером глубины ---
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = ctx.swapchainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkFormat depthFormat = findDepthFormat(ctx.physicalDevice);
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = depthFormat;
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        vkCreateRenderPass(ctx.device, &renderPassInfo, nullptr, &ctx.renderPass);

        // --- Descriptor Set Layout ---
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &uboLayoutBinding;
        vkCreateDescriptorSetLayout(ctx.device, &layoutInfo, nullptr, &ctx.descriptorSetLayout);

        // --- Graphics Pipeline ---
        auto vertShaderCode = readFile("vert.spv");
        auto fragShaderCode = readFile("frag.spv");
        VkShaderModule vertShaderModule = createShaderModule(ctx.device, vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(ctx.device, fragShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";
        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)ctx.swapchainExtent.width;
        viewport.height = (float)ctx.swapchainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = ctx.swapchainExtent;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &ctx.descriptorSetLayout;
        vkCreatePipelineLayout(ctx.device, &pipelineLayoutInfo, nullptr, &ctx.pipelineLayout);

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.layout = ctx.pipelineLayout;
        pipelineInfo.renderPass = ctx.renderPass;
        pipelineInfo.subpass = 0;

        vkCreateGraphicsPipelines(ctx.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &ctx.graphicsPipeline);
        vkDestroyShaderModule(ctx.device, fragShaderModule, nullptr);
        vkDestroyShaderModule(ctx.device, vertShaderModule, nullptr);

        // --- Command Pool ---
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = ctx.graphicsFamilyIndex;
        vkCreateCommandPool(ctx.device, &poolInfo, nullptr, &ctx.commandPool);

        // --- Depth Buffer Image ---
        createImage(ctx, ctx.swapchainExtent.width, ctx.swapchainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, ctx.depthImage, ctx.depthImageMemory);
        ctx.depthImageView = createImageView(ctx.device, ctx.depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

        // --- Framebuffers ---
        ctx.swapchainFramebuffers.resize(ctx.swapchainImageViews.size());
        for (size_t i = 0; i < ctx.swapchainImageViews.size(); i++) {
            std::array<VkImageView, 2> fbAttachments = { ctx.swapchainImageViews[i], ctx.depthImageView };
            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = ctx.renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(fbAttachments.size());
            framebufferInfo.pAttachments = fbAttachments.data();
            framebufferInfo.width = ctx.swapchainExtent.width;
            framebufferInfo.height = ctx.swapchainExtent.height;
            framebufferInfo.layers = 1;
            vkCreateFramebuffer(ctx.device, &framebufferInfo, nullptr, &ctx.swapchainFramebuffers[i]);
        }

        // --- Vertex Buffer ---
        VkDeviceSize vBufferSize = sizeof(vertices[0]) * vertices.size();
        VkBuffer vStagingBuffer;
        VkDeviceMemory vStagingBufferMemory;
        createBuffer(ctx, vBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vStagingBuffer, vStagingBufferMemory);
        void* vData;
        vkMapMemory(ctx.device, vStagingBufferMemory, 0, vBufferSize, 0, &vData);
        memcpy(vData, vertices.data(), (size_t)vBufferSize);
        vkUnmapMemory(ctx.device, vStagingBufferMemory);
        createBuffer(ctx, vBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, ctx.vertexBuffer, ctx.vertexBufferMemory);
        copyBuffer(ctx, vStagingBuffer, ctx.vertexBuffer, vBufferSize);
        vkDestroyBuffer(ctx.device, vStagingBuffer, nullptr);
        vkFreeMemory(ctx.device, vStagingBufferMemory, nullptr);

        // --- Index Buffer ---
        VkDeviceSize iBufferSize = sizeof(indices[0]) * indices.size();
        VkBuffer iStagingBuffer;
        VkDeviceMemory iStagingBufferMemory;
        createBuffer(ctx, iBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, iStagingBuffer, iStagingBufferMemory);
        void* iData;
        vkMapMemory(ctx.device, iStagingBufferMemory, 0, iBufferSize, 0, &iData);
        memcpy(iData, indices.data(), (size_t)iBufferSize);
        vkUnmapMemory(ctx.device, iStagingBufferMemory);
        createBuffer(ctx, iBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, ctx.indexBuffer, ctx.indexBufferMemory);
        copyBuffer(ctx, iStagingBuffer, ctx.indexBuffer, iBufferSize);
        vkDestroyBuffer(ctx.device, iStagingBuffer, nullptr);
        vkFreeMemory(ctx.device, iStagingBufferMemory, nullptr);

        // --- Uniform Buffers ---
        VkDeviceSize uBufferSize = sizeof(UniformBufferObject);
        ctx.uniformBuffers.resize(VulkanContext::MAX_FRAMES_IN_FLIGHT);
        ctx.uniformBuffersMemory.resize(VulkanContext::MAX_FRAMES_IN_FLIGHT);
        ctx.uniformBuffersMapped.resize(VulkanContext::MAX_FRAMES_IN_FLIGHT);
        for (size_t i = 0; i < VulkanContext::MAX_FRAMES_IN_FLIGHT; i++) {
            createBuffer(ctx, uBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, ctx.uniformBuffers[i], ctx.uniformBuffersMemory[i]);
            vkMapMemory(ctx.device, ctx.uniformBuffersMemory[i], 0, uBufferSize, 0, &ctx.uniformBuffersMapped[i]);
        }

        // --- Descriptor Pool & Sets ---
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = static_cast<uint32_t>(VulkanContext::MAX_FRAMES_IN_FLIGHT);
        VkDescriptorPoolCreateInfo poolInfoCreate{};
        poolInfoCreate.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfoCreate.poolSizeCount = 1;
        poolInfoCreate.pPoolSizes = &poolSize;
        poolInfoCreate.maxSets = static_cast<uint32_t>(VulkanContext::MAX_FRAMES_IN_FLIGHT);
        vkCreateDescriptorPool(ctx.device, &poolInfoCreate, nullptr, &ctx.descriptorPool);

        std::vector<VkDescriptorSetLayout> layouts(VulkanContext::MAX_FRAMES_IN_FLIGHT, ctx.descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocSetInfo{};
        allocSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocSetInfo.descriptorPool = ctx.descriptorPool;
        allocSetInfo.descriptorSetCount = static_cast<uint32_t>(VulkanContext::MAX_FRAMES_IN_FLIGHT);
        allocSetInfo.pSetLayouts = layouts.data();
        ctx.descriptorSets.resize(VulkanContext::MAX_FRAMES_IN_FLIGHT);
        vkAllocateDescriptorSets(ctx.device, &allocSetInfo, ctx.descriptorSets.data());

        for (size_t i = 0; i < VulkanContext::MAX_FRAMES_IN_FLIGHT; i++) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = ctx.uniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);
            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = ctx.descriptorSets[i];
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;
            vkUpdateDescriptorSets(ctx.device, 1, &descriptorWrite, 0, nullptr);
        }

        ctx.commandBuffers.resize(VulkanContext::MAX_FRAMES_IN_FLIGHT);
        VkCommandBufferAllocateInfo cmdAllocInfo{};
        cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdAllocInfo.commandPool = ctx.commandPool;
        cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmdAllocInfo.commandBufferCount = (uint32_t)ctx.commandBuffers.size();
        vkAllocateCommandBuffers(ctx.device, &cmdAllocInfo, ctx.commandBuffers.data());

        ctx.imageAvailableSemaphores.resize(VulkanContext::MAX_FRAMES_IN_FLIGHT);
        ctx.renderFinishedSemaphores.resize(VulkanContext::MAX_FRAMES_IN_FLIGHT);
        ctx.inFlightFences.resize(VulkanContext::MAX_FRAMES_IN_FLIGHT);
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        for (size_t i = 0; i < VulkanContext::MAX_FRAMES_IN_FLIGHT; i++) {
            vkCreateSemaphore(ctx.device, &semaphoreInfo, nullptr, &ctx.imageAvailableSemaphores[i]);
            vkCreateSemaphore(ctx.device, &semaphoreInfo, nullptr, &ctx.renderFinishedSemaphores[i]);
            vkCreateFence(ctx.device, &fenceInfo, nullptr, &ctx.inFlightFences[i]);
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
    return true;
}

bool drawFrame(VulkanContext& ctx) {
    vkWaitForFences(ctx.device, 1, &ctx.inFlightFences[ctx.currentFrame], VK_TRUE, UINT64_MAX);
    uint32_t imageIndex;
    vkAcquireNextImageKHR(ctx.device, ctx.swapchain, UINT64_MAX, ctx.imageAvailableSemaphores[ctx.currentFrame], VK_NULL_HANDLE, &imageIndex);
    
    updateUniformBuffer(ctx, ctx.currentFrame); // Обновляем матрицы
    
    vkResetFences(ctx.device, 1, &ctx.inFlightFences[ctx.currentFrame]);
    vkResetCommandBuffer(ctx.commandBuffers[ctx.currentFrame], 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(ctx.commandBuffers[ctx.currentFrame], &beginInfo);

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = ctx.renderPass;
    renderPassInfo.framebuffer = ctx.swapchainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = ctx.swapchainExtent;

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(ctx.commandBuffers[ctx.currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(ctx.commandBuffers[ctx.currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, ctx.graphicsPipeline);
    
    VkBuffer vertexBuffers[] = {ctx.vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(ctx.commandBuffers[ctx.currentFrame], 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(ctx.commandBuffers[ctx.currentFrame], ctx.indexBuffer, 0, VK_INDEX_TYPE_UINT16);
    
    vkCmdBindDescriptorSets(ctx.commandBuffers[ctx.currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, ctx.pipelineLayout, 0, 1, &ctx.descriptorSets[ctx.currentFrame], 0, nullptr);
    vkCmdDrawIndexed(ctx.commandBuffers[ctx.currentFrame], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
    
    vkCmdEndRenderPass(ctx.commandBuffers[ctx.currentFrame]);
    vkEndCommandBuffer(ctx.commandBuffers[ctx.currentFrame]);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkSemaphore waitSemaphores[] = {ctx.imageAvailableSemaphores[ctx.currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &ctx.commandBuffers[ctx.currentFrame];
    VkSemaphore signalSemaphores[] = {ctx.renderFinishedSemaphores[ctx.currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkQueueSubmit(ctx.graphicsQueue, 1, &submitInfo, ctx.inFlightFences[ctx.currentFrame]);

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapchains[] = {ctx.swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;
    vkQueuePresentKHR(ctx.presentQueue, &presentInfo);

    ctx.currentFrame = (ctx.currentFrame + 1) % VulkanContext::MAX_FRAMES_IN_FLIGHT;
    return true;
}

void cleanupVulkan(VulkanContext& ctx) {
    vkDeviceWaitIdle(ctx.device);
    for (size_t i = 0; i < VulkanContext::MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(ctx.device, ctx.renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(ctx.device, ctx.imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(ctx.device, ctx.inFlightFences[i], nullptr);
    }
    for (size_t i = 0; i < VulkanContext::MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(ctx.device, ctx.uniformBuffers[i], nullptr);
        vkFreeMemory(ctx.device, ctx.uniformBuffersMemory[i], nullptr);
    }
    vkDestroyDescriptorPool(ctx.device, ctx.descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(ctx.device, ctx.descriptorSetLayout, nullptr);
    vkDestroyBuffer(ctx.device, ctx.indexBuffer, nullptr);
    vkFreeMemory(ctx.device, ctx.indexBufferMemory, nullptr);
    vkDestroyBuffer(ctx.device, ctx.vertexBuffer, nullptr);
    vkFreeMemory(ctx.device, ctx.vertexBufferMemory, nullptr);
    vkDestroyImageView(ctx.device, ctx.depthImageView, nullptr);
    vkDestroyImage(ctx.device, ctx.depthImage, nullptr);
    vkFreeMemory(ctx.device, ctx.depthImageMemory, nullptr);
    vkDestroyCommandPool(ctx.device, ctx.commandPool, nullptr);
    for (auto framebuffer : ctx.swapchainFramebuffers) vkDestroyFramebuffer(ctx.device, framebuffer, nullptr);
    vkDestroyPipeline(ctx.device, ctx.graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(ctx.device, ctx.pipelineLayout, nullptr);
    vkDestroyRenderPass(ctx.device, ctx.renderPass, nullptr);
    for (auto imageView : ctx.swapchainImageViews) vkDestroyImageView(ctx.device, imageView, nullptr);
    vkDestroySwapchainKHR(ctx.device, ctx.swapchain, nullptr);
    vkDestroyDevice(ctx.device, nullptr);
    vkDestroySurfaceKHR(ctx.instance, ctx.surface, nullptr);
    vkDestroyInstance(ctx.instance, nullptr);
}