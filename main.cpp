#define SDL_MAIN_HANDLED
#include "Headers/Engine.hpp"
#include "Headers/log.hpp"
#include <iostream>

class Test : public Engine {
private:
    RenderObject* playerCube;
    RenderObject* floatingSphere;

protected:
    void start() override {
        Logger::log(Logger::LogLevel::INFO, "Setting up the scene...");
        camera.position = glm::vec3(0.0f, 2.0f, 6.0f);

        RenderObject* floor = addPrimitive(Primitive::createCube(), glm::vec3(0.0f, -0.5f, 0.0f));
        floor->transform.scale = glm::vec3(10.0f, 0.1f, 10.0f);

        playerCube = addPrimitive(Primitive::createCube(), glm::vec3(0.0f, 0.5f, 0.0f));
        floatingSphere = addPrimitive(Primitive::createSphere(32, 32), glm::vec3(2.0f, 1.0f, 0.0f));
    }
    
    // NEW: Listen for singular key presses
    void onKeyDown(SDL_Scancode key) override {
        // Press Escape to toggle the cursor and freeze the camera
        if (key == SDL_SCANCODE_ESCAPE) {
            setMouseLock(!getMouseLock());
            
            if (getMouseLock()) {
                Logger::log(Logger::LogLevel::INFO, "Game Resumed.");
            } else {
                Logger::log(Logger::LogLevel::INFO, "Game Paused. Cursor freed.");
            }
        }
    }

    void update(float deltaTime) override {
        float moveSpeed = 3.0f * deltaTime;
        float rotateSpeed = 90.0f * deltaTime;

        floatingSphere->transform.rotation.y += rotateSpeed;
        floatingSphere->transform.rotation.z += rotateSpeed * 0.5f;

        if (isKeyPressed(SDL_SCANCODE_I)) playerCube->transform.position.z -= moveSpeed;
        if (isKeyPressed(SDL_SCANCODE_K)) playerCube->transform.position.z += moveSpeed;
        if (isKeyPressed(SDL_SCANCODE_J)) playerCube->transform.position.x -= moveSpeed;
        if (isKeyPressed(SDL_SCANCODE_L)) playerCube->transform.position.x += moveSpeed;
    }
};

int main(int argc, char* argv[]) {
    try {
        Test game;
        game.run();
    } catch (const std::exception& e) {
        Logger::log(Logger::LogLevel::ERROR, std::string("Fatal exception: ") + e.what());
        return -1;
    }
    return 0;
}
