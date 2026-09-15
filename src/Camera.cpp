#include "camel/Camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <stdexcept>

Camera::Camera(glm::vec3 startPosition)
    : position(startPosition),
      front(0.0f, 0.0f, -1.0f),
      up(0.0f, 1.0f, 0.0f),
      right(1.0f, 0.0f, 0.0f),
      worldUp(0.0f, 1.0f, 0.0f),
      yaw(-90.0f),
      pitch(0.0f),
      movementSpeed(5.0f),
      mouseSensitivity(0.1f) {
    updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position, position + front, up);
}

void Camera::processKeyboard(const bool* state, float deltaTime) {
    if (state == nullptr || deltaTime <= 0.0f) {
        return;
    }

    const float velocity = movementSpeed * deltaTime;
    if (state[SDL_SCANCODE_W]) position += front * velocity;
    if (state[SDL_SCANCODE_S]) position -= front * velocity;
    if (state[SDL_SCANCODE_A]) position -= right * velocity;
    if (state[SDL_SCANCODE_D]) position += right * velocity;
    if (state[SDL_SCANCODE_SPACE]) position += worldUp * velocity;
    if (state[SDL_SCANCODE_LCTRL]) position -= worldUp * velocity;
}

void Camera::processMouseMovement(float xoffset, float yoffset) {
    yaw -= xoffset * mouseSensitivity;
    pitch += yoffset * mouseSensitivity;
    pitch = glm::clamp(pitch, -89.0f, 89.0f);
    updateCameraVectors();
}

void Camera::setDirection(glm::vec3 direction) {
    const float length = glm::length(direction);
    if (length <= 0.0f) {
        throw std::invalid_argument("Camera direction cannot be zero");
    }

    direction /= length;
    pitch = glm::degrees(std::asin(glm::clamp(direction.y, -1.0f, 1.0f)));
    yaw = glm::degrees(std::atan2(direction.z, direction.x));
    updateCameraVectors();
}

void Camera::updateCameraVectors() {
    const glm::vec3 direction{
        std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch)),
        std::sin(glm::radians(pitch)),
        std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch))
    };

    front = glm::normalize(direction);
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}
