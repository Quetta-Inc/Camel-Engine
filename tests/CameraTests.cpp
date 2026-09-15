#include "camel/Camera.hpp"

#include <array>
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
        Camera camera;
        expect(glm::distance(camera.position, {0.0f, 0.0f, 3.0f}) < 0.0001f,
               "Camera default position is incorrect");
        expect(glm::distance(camera.front, {0.0f, 0.0f, -1.0f}) < 0.0001f,
               "Camera default direction is incorrect");

        camera.setDirection({1.0f, 0.0f, 0.0f});
        expect(glm::distance(camera.front, {1.0f, 0.0f, 0.0f}) < 0.0001f,
               "Camera direction setter failed");
        expect(std::abs(glm::length(camera.right) - 1.0f) < 0.0001f,
               "Camera right vector is not normalized");

        camera.processMouseMovement(0.0f, 2000.0f);
        expect(camera.pitch <= 89.0f, "Camera pitch exceeded its upper limit");
        camera.processMouseMovement(0.0f, -4000.0f);
        expect(camera.pitch >= -89.0f, "Camera pitch exceeded its lower limit");

        std::array<bool, SDL_SCANCODE_COUNT> keys{};
        camera.position = {0.0f, 0.0f, 0.0f};
        camera.setDirection({0.0f, 0.0f, -1.0f});
        keys[SDL_SCANCODE_W] = true;
        camera.processKeyboard(keys.data(), 1.0f);
        expect(camera.position.z < -4.9f, "W should move the camera forward");

        const glm::vec3 positionBefore = camera.position;
        camera.processKeyboard(nullptr, 1.0f);
        camera.processKeyboard(keys.data(), 0.0f);
        expect(glm::distance(camera.position, positionBefore) < 0.0001f,
               "Invalid keyboard input should not move the camera");
    } catch (const std::exception& exception) {
        std::cerr << exception.what() << '\n';
        return 1;
    }

    return 0;
}
