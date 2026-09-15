#include "camel/Scene.hpp"

#include <cmath>
#include <exception>
#include <iostream>
#include <stdexcept>

namespace {

void expect(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

} // namespace

int main() {
    try {
        Transform transform;
        transform.position = {2.0f, 3.0f, 4.0f};
        transform.scale = {2.0f, 3.0f, 4.0f};

        const glm::vec4 transformed = transform.getModelMatrix() * glm::vec4(1.0f);
        expect(glm::distance(glm::vec3(transformed), {4.0f, 6.0f, 8.0f}) < 0.0001f,
               "Transform did not apply scale and translation");

        transform = {};
        expect(glm::distance(
                   glm::vec3(transform.getModelMatrix() * glm::vec4(1.0f)),
                   {1.0f, 1.0f, 1.0f}
               ) < 0.0001f,
               "Transform defaults are incorrect");
    } catch (const std::exception& exception) {
        std::cerr << exception.what() << '\n';
        return 1;
    }

    return 0;
}
