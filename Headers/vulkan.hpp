#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord; 
};

struct UniformBufferObject {
    glm::mat4 view;
    glm::mat4 proj;
};
