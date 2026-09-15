#define SDL_MAIN_HANDLED

#include "camel/Engine.hpp"
#include "camel/Logger.hpp"

#include <SDL3/SDL.h>
#include <glm/glm.hpp>

#include <array>
#include <exception>
#include <string>
#include <vector>

namespace {

class CubeCollector final : public Engine {
public:
    CubeCollector()
        : spawnPoints{
              glm::vec3{-3.0f, 0.0f, -3.0f},
              glm::vec3{3.0f, 0.0f, -3.0f},
              glm::vec3{0.0f, 0.0f, -6.0f},
              glm::vec3{4.0f, 0.0f, -7.0f}
          } {}

protected:
    void start() override {
        camera.position = {0.0f, 3.5f, 8.0f};
        camera.setDirection({0.0f, -0.25f, -1.0f});

        player = addPrimitive(Primitive::createCube(), {0.0f, 0.0f, 0.0f});
        player->transform.scale = {0.8f, 0.8f, 0.8f};

        for (const glm::vec3& point : spawnPoints) {
            collectibles.push_back(addPrimitive(Primitive::createCube(), point));
        }

        Logger::log(
            Logger::LogLevel::Info,
            "Cube Collector started. Move with I/J/K/L, collect every cube, press R to reset."
        );
    }

    void onKeyDown(SDL_Scancode key) override {
        if (key == SDL_SCANCODE_ESCAPE) {
            setMouseLock(!getMouseLock());
            Logger::log(
                Logger::LogLevel::Info,
                getMouseLock()
                    ? "Camera control enabled."
                    : "Camera control paused."
            );
        } else if (key == SDL_SCANCODE_R) {
            resetGame();
        }
    }

    void update(float deltaTime) override {
        if (player == nullptr) {
            return;
        }

        movePlayer(deltaTime);
        player->transform.rotation.y += 90.0f * deltaTime;

        for (size_t index = 0; index < collectibles.size(); ++index) {
            RenderObject* collectible = collectibles[index];
            if (collectible->transform.scale.x <= 0.0f) {
                continue;
            }

            collectible->transform.rotation.y += 70.0f * deltaTime;
            collectible->transform.rotation.x += 35.0f * deltaTime;

            const glm::vec2 playerPosition{player->transform.position.x,
                                           player->transform.position.z};
            const glm::vec2 collectiblePosition{
                collectible->transform.position.x,
                collectible->transform.position.z
            };
            if (glm::distance(playerPosition, collectiblePosition) < 0.9f) {
                collectible->transform.scale = {0.0f, 0.0f, 0.0f};
                ++score;
                Logger::log(
                    Logger::LogLevel::Info,
                    "Cube collected: " + std::to_string(score) + "/" +
                        std::to_string(collectibles.size())
                );
            }
        }

        if (score == collectibles.size() && !gameCompleted) {
            gameCompleted = true;
            Logger::log(
                Logger::LogLevel::Info,
                "You win! Press R to play again."
            );
        }
    }

private:
    void movePlayer(float deltaTime) {
        const float speed = 3.5f * deltaTime;
        glm::vec3& position = player->transform.position;

        if (isKeyPressed(SDL_SCANCODE_I)) position.z -= speed;
        if (isKeyPressed(SDL_SCANCODE_K)) position.z += speed;
        if (isKeyPressed(SDL_SCANCODE_J)) position.x -= speed;
        if (isKeyPressed(SDL_SCANCODE_L)) position.x += speed;

        position.x = glm::clamp(position.x, -6.0f, 6.0f);
        position.z = glm::clamp(position.z, -9.0f, 2.0f);
    }

    void resetGame() {
        if (player == nullptr) {
            return;
        }

        player->transform.position = {0.0f, 0.0f, 0.0f};
        player->transform.rotation = {0.0f, 0.0f, 0.0f};
        score = 0;
        gameCompleted = false;

        for (size_t index = 0; index < collectibles.size(); ++index) {
            collectibles[index]->transform.position = spawnPoints[index];
            collectibles[index]->transform.rotation = {0.0f, 0.0f, 0.0f};
            collectibles[index]->transform.scale = {1.0f, 1.0f, 1.0f};
        }

        Logger::log(Logger::LogLevel::Info, "Game reset.");
    }

    RenderObject* player = nullptr;
    std::array<glm::vec3, 4> spawnPoints;
    std::vector<RenderObject*> collectibles;
    size_t score = 0;
    bool gameCompleted = false;
};

} // namespace

int main(int argc, char* argv[]) {
    const bool smokeTest = argc > 1 && std::string(argv[1]) == "--smoke-test";

    try {
        CubeCollector game;
        game.run(smokeTest);
    } catch (const std::exception& exception) {
        Logger::log(
            Logger::LogLevel::Error,
            std::string("Fatal exception: ") + exception.what()
        );
        return 1;
    }

    return 0;
}
