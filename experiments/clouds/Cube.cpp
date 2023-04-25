//
// Created by Kristoffer Borgstrøm on 23/03/2023.
//

#include "Cube.hpp"

#include <ituGL/geometry/VertexFormat.h>

std::shared_ptr<Mesh> Cube::InitializeQuad() {

    std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();

    struct Vertex
    {
        Vertex() = default;
        Vertex(const glm::vec3& position, const glm::vec2 texCoord)
                : position(position), texCoord(texCoord) {}
        glm::vec3 position;
        glm::vec2 texCoord;
    };

    VertexFormat vertexFormat;
    vertexFormat.AddVertexAttribute<float>(3);
    vertexFormat.AddVertexAttribute<float>(2);

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // Tedious cube mesh creation

    // Bottom left front  - 0
    vertices.emplace_back(glm::vec3(-1.0f, -1.0f, 1.0f), glm::vec2(0.0f, 0.0f));
    // Bottom right front - 1
    vertices.emplace_back(glm::vec3(1.0f, -1.0f, 1.0f), glm::vec2(1.0f, 0.0f));
    // Bottom left back   - 2
    vertices.emplace_back(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec2(0.0f, 1.0f));
    // Bottom right back  - 3
    vertices.emplace_back(glm::vec3(1.0f, -1.0f, -1.0f), glm::vec2(1.0f, 1.0f));

    // Top left front     - 4
    vertices.emplace_back(glm::vec3(-1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f));
    // Top right front    - 5
    vertices.emplace_back(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f));
    // Top left back      - 6
    vertices.emplace_back(glm::vec3(-1.0f, 1.0f, -1.0f), glm::vec2(0.0f, 1.0f));
    // Top right back     - 7
    vertices.emplace_back(glm::vec3(1.0f, 1.0f, -1.0f), glm::vec2(1.0f, 1.0f));

    // Bottom
    indices.push_back(0);
    indices.push_back(3);
    indices.push_back(1);

    indices.push_back(0);
    indices.push_back(2);
    indices.push_back(3);

    // Front
    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(5);

    indices.push_back(0);
    indices.push_back(5);
    indices.push_back(4);

    // Right
    indices.push_back(1);
    indices.push_back(3);
    indices.push_back(7);

    indices.push_back(1);
    indices.push_back(7);
    indices.push_back(5);

    // Back
    indices.push_back(3);
    indices.push_back(2);
    indices.push_back(6);

    indices.push_back(3);
    indices.push_back(6);
    indices.push_back(7);

    // Left
    indices.push_back(2);
    indices.push_back(0);
    indices.push_back(4);

    indices.push_back(2);
    indices.push_back(4);
    indices.push_back(6);

    // Top
    indices.push_back(4);
    indices.push_back(5);
    indices.push_back(7);

    indices.push_back(4);
    indices.push_back(7);
    indices.push_back(6);

    mesh->AddSubmesh<Vertex, unsigned int, VertexFormat::LayoutIterator>(Drawcall::Primitive::Triangles, vertices, indices,
                                                                         vertexFormat.LayoutBegin(static_cast<int>(vertices.size()), true), vertexFormat.LayoutEnd());
    return mesh;
}
