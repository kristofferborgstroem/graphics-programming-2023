#pragma once

#include <ituGL/application/Application.h>
#include <ituGL/geometry/VertexBufferObject.h>
#include <ituGL/geometry/VertexArrayObject.h>
#include <ituGL/geometry/ElementBufferObject.h>
#include <cmath>
#include <vector>

// (todo) 01.1: Include the libraries you need


class Particles : public Application
{
public:
    Particles();

protected:
    void Initialize() override;
    void Update() override;
    void Render() override;
    void Cleanup() override;

private:
    void BuildShaders();
    void UpdateOutputMode();

private:
    unsigned int m_gridX, m_gridY;
    unsigned int m_shaderProgram;

    // (todo) 01.1: Declare an VBO, VAO
    VertexBufferObject vbo;
    VertexArrayObject vao;


    // (todo) 01.5: Declare an EBO
    ElementBufferObject ebo;
    int indicesSize = 0;

    struct Vector2
    {
        Vector2() : Vector2(0.f, 0.f) {}
        Vector2(float x, float y) : x(x), y(y) {}
        float x, y;
    };

    struct Vector3
    {
        Vector3() : Vector3(0.f,0.f,0.f) {}
        Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
        float x, y, z;

        Vector3 Normalize() const
        {
            float length = std::sqrt(1 + x * x + y * y);
            return Vector3(x / length, y / length, z / length);
        }
    };

    struct Vertex {
        Vector3 position;
    };

    std::vector<Vertex> vertices;

};
