#include "camel/Primitive.hpp"

#include <cmath>
#include <limits>
#include <stdexcept>

namespace {

constexpr float pi = 3.14159265358979323846f;

} // namespace

MeshData Primitive::createCube() {
    MeshData mesh;
    const glm::vec3 whiteColor{1.0f, 1.0f, 1.0f};
    mesh.vertices = {
        {{-0.5f, -0.5f,  0.5f}, whiteColor, {0.0f, 0.0f}},
        {{ 0.5f, -0.5f,  0.5f}, whiteColor, {1.0f, 0.0f}},
        {{ 0.5f,  0.5f,  0.5f}, whiteColor, {1.0f, 1.0f}},
        {{-0.5f,  0.5f,  0.5f}, whiteColor, {0.0f, 1.0f}},
        {{ 0.5f, -0.5f,  0.5f}, whiteColor, {0.0f, 0.0f}},
        {{ 0.5f, -0.5f, -0.5f}, whiteColor, {1.0f, 0.0f}},
        {{ 0.5f,  0.5f, -0.5f}, whiteColor, {1.0f, 1.0f}},
        {{ 0.5f,  0.5f,  0.5f}, whiteColor, {0.0f, 1.0f}},
        {{ 0.5f, -0.5f, -0.5f}, whiteColor, {0.0f, 0.0f}},
        {{-0.5f, -0.5f, -0.5f}, whiteColor, {1.0f, 0.0f}},
        {{-0.5f,  0.5f, -0.5f}, whiteColor, {1.0f, 1.0f}},
        {{ 0.5f,  0.5f, -0.5f}, whiteColor, {0.0f, 1.0f}},
        {{-0.5f, -0.5f, -0.5f}, whiteColor, {0.0f, 0.0f}},
        {{-0.5f, -0.5f,  0.5f}, whiteColor, {1.0f, 0.0f}},
        {{-0.5f,  0.5f,  0.5f}, whiteColor, {1.0f, 1.0f}},
        {{-0.5f,  0.5f, -0.5f}, whiteColor, {0.0f, 1.0f}},
        {{-0.5f, -0.5f, -0.5f}, whiteColor, {0.0f, 0.0f}},
        {{ 0.5f, -0.5f, -0.5f}, whiteColor, {1.0f, 0.0f}},
        {{ 0.5f, -0.5f,  0.5f}, whiteColor, {1.0f, 1.0f}},
        {{-0.5f, -0.5f,  0.5f}, whiteColor, {0.0f, 1.0f}},
        {{-0.5f,  0.5f,  0.5f}, whiteColor, {0.0f, 0.0f}},
        {{ 0.5f,  0.5f,  0.5f}, whiteColor, {1.0f, 0.0f}},
        {{ 0.5f,  0.5f, -0.5f}, whiteColor, {1.0f, 1.0f}},
        {{-0.5f,  0.5f, -0.5f}, whiteColor, {0.0f, 1.0f}}
    };
    mesh.indices = {
        0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4,
        8, 9, 10, 10, 11, 8, 12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23, 20
    };
    return mesh;
}

MeshData Primitive::createSphere(int sectorCount, int stackCount) {
    if (sectorCount < 3 || stackCount < 2) {
        throw std::invalid_argument("A sphere needs at least 3 sectors and 2 stacks");
    }

    const auto vertexCount = static_cast<uint64_t>(sectorCount + 1) *
                             static_cast<uint64_t>(stackCount + 1);
    if (vertexCount > static_cast<uint64_t>(std::numeric_limits<uint16_t>::max()) + 1) {
        throw std::invalid_argument("Sphere has too many vertices for uint16 indices");
    }

    MeshData mesh;
    mesh.vertices.reserve(static_cast<size_t>(vertexCount));
    mesh.indices.reserve(static_cast<size_t>(sectorCount * stackCount * 6));

    constexpr float radius = 0.5f;
    const float sectorStep = 2.0f * pi / static_cast<float>(sectorCount);
    const float stackStep = pi / static_cast<float>(stackCount);

    for (int stack = 0; stack <= stackCount; ++stack) {
        const float stackAngle = pi / 2.0f - static_cast<float>(stack) * stackStep;
        const float xy = radius * std::cos(stackAngle);
        const float z = radius * std::sin(stackAngle);

        for (int sector = 0; sector <= sectorCount; ++sector) {
            const float sectorAngle = static_cast<float>(sector) * sectorStep;
            mesh.vertices.push_back({
                {xy * std::cos(sectorAngle), xy * std::sin(sectorAngle), z},
                {1.0f, 1.0f, 1.0f},
                {
                    static_cast<float>(sector) / static_cast<float>(sectorCount),
                    static_cast<float>(stack) / static_cast<float>(stackCount)
                }
            });
        }
    }

    for (int stack = 0; stack < stackCount; ++stack) {
        int first = stack * (sectorCount + 1);
        int second = first + sectorCount + 1;
        for (int sector = 0; sector < sectorCount; ++sector, ++first, ++second) {
            if (stack != 0) {
                mesh.indices.push_back(static_cast<uint16_t>(first));
                mesh.indices.push_back(static_cast<uint16_t>(second));
                mesh.indices.push_back(static_cast<uint16_t>(first + 1));
            }
            if (stack != stackCount - 1) {
                mesh.indices.push_back(static_cast<uint16_t>(first + 1));
                mesh.indices.push_back(static_cast<uint16_t>(second));
                mesh.indices.push_back(static_cast<uint16_t>(second + 1));
            }
        }
    }

    return mesh;
}
