#pragma once

#include "VulkanBuffer.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cstdint>
#include <memory>

struct Transform {
    glm::vec3 position{0.0f};
    glm::vec3 rotation{0.0f};
    glm::vec3 scale{1.0f};

    glm::mat4 getModelMatrix() const {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
        model = glm::rotate(model, glm::radians(rotation.x), {1.0f, 0.0f, 0.0f});
        model = glm::rotate(model, glm::radians(rotation.y), {0.0f, 1.0f, 0.0f});
        model = glm::rotate(model, glm::radians(rotation.z), {0.0f, 0.0f, 1.0f});
        return glm::scale(model, scale);
    }
};

struct RenderObject {
    std::unique_ptr<VulkanBuffer> vertexBuffer;
    std::unique_ptr<VulkanBuffer> indexBuffer;
    uint32_t indexCount = 0;
    Transform transform;
};
