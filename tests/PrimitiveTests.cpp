#include "camel/Primitive.hpp"

#include <cmath>
#include <cstdint>
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
        const MeshData cube = Primitive::createCube();
        expect(cube.vertices.size() == 24, "Cube should have 24 face vertices");
        expect(cube.indices.size() == 36, "Cube should have 36 indices");
        for (uint16_t index : cube.indices) {
            expect(index < cube.vertices.size(), "Cube index is out of range");
        }

        const MeshData sphere = Primitive::createSphere(16, 8);
        expect(sphere.vertices.size() == 17 * 9, "Sphere vertex grid is incorrect");
        expect(sphere.indices.size() == 16 * 7 * 6, "Sphere index count is incorrect");
        for (const Vertex& vertex : sphere.vertices) {
            expect(std::abs(glm::length(vertex.pos) - 0.5f) < 0.0001f,
                   "Sphere vertex must lie on the requested radius");
            expect(vertex.texCoord.x >= 0.0f && vertex.texCoord.x <= 1.0f,
                   "Sphere U coordinate is outside [0, 1]");
            expect(vertex.texCoord.y >= 0.0f && vertex.texCoord.y <= 1.0f,
                   "Sphere V coordinate is outside [0, 1]");
        }

        bool rejected = false;
        try {
            (void)Primitive::createSphere(2, 1);
        } catch (const std::invalid_argument&) {
            rejected = true;
        }
        expect(rejected, "Invalid sphere dimensions must be rejected");
    } catch (const std::exception& exception) {
        std::cerr << exception.what() << '\n';
        return 1;
    }

    return 0;
}
