#pragma once

#include <SDL3/SDL.h>
#include <glm/glm.hpp>

class Camera {
public:
    explicit Camera(glm::vec3 startPosition = {0.0f, 0.0f, 3.0f});

    glm::mat4 getViewMatrix() const;
    void processKeyboard(const bool* state, float deltaTime);
    void processMouseMovement(float xoffset, float yoffset);
    void setDirection(glm::vec3 direction);

    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    float yaw;
    float pitch;
    float movementSpeed;
    float mouseSensitivity;

private:
    void updateCameraVectors();
};
