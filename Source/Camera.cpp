#include "Headers/Camera.hpp"

Camera::Camera(glm::vec3 startPosition)
    : position(startPosition), worldUp(glm::vec3(0.0f, -1.0f, 0.0f)), yaw(-90.0f), pitch(0.0f),
      movementSpeed(5.0f), mouseSensitivity(0.1f) {
    updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position, position + front, up);
}

// Change const uint8_t* to const bool*
void Camera::processKeyboard(const bool* state, float deltaTime) {
    float velocity = movementSpeed * deltaTime;
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

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    updateCameraVectors();
}

void Camera::updateCameraVectors() {
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(newFront);
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}
