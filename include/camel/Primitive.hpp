#pragma once

#include "VulkanTypes.hpp"

#include <cstdint>
#include <vector>

struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;
};

class Primitive {
public:
    static MeshData createCube();
    static MeshData createSphere(int sectorCount, int stackCount);
};
