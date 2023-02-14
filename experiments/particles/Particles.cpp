#include "Particles.h"

// (todo) 01.1: Include the libraries you need

#include <ituGL/core/Color.h>
#include <ituGL/geometry/VertexAttribute.h>
#include <iostream>


#define STB_PERLIN_IMPLEMENTATION
#include <stb_perlin.h>

// Helper structures. Declared here only for this exercise

// (todo) 01.8: Declare an struct with the vertex format



Particles::Particles()
    : Application(800, 800, "Terrain demo"), m_gridX(16), m_gridY(16), m_shaderProgram(0)
{
}

void Particles::Initialize()
{
    Application::Initialize();

    // Build shaders and store in m_shaderProgram
    BuildShaders();

    // (todo) 01.1: Create containers for the vertex position
    std::vector<Vertex> temp;
    std::vector<int> indices;

    // (todo) 01.1: Fill in vertex data
    /*
     Vertex position data layout

     2 ----- 3 ----- 5
     |       |       |
     |       |       |
     0 ----- 1 ----- 4

     */
    float xoffset = 1.0f / (float) m_gridX;
    float yoffset = 1.0f / (float) m_gridY;

    for (int x = 0; x <= m_gridX; ++x) {
        for (int y = 0; y <= m_gridY; ++y) {
            Vector3 position = {((float) x * xoffset) - 0.5f, ((float) y * yoffset) - 0.5f, 0.0f};
            vertices.push_back({ position });
        }
    }

    int offset = 0;
    int width = m_gridX + 1;
    for (int x = 0; x <= m_gridX - 1; ++x) {
        for (int i = 0; i <= m_gridY - 1; ++i) {
            indices.push_back(offset);
            indices.push_back(offset + 1);
            indices.push_back(offset + width);

            indices.push_back(offset + 1);
            indices.push_back(offset + 1 + width);
            indices.push_back(offset + width);

            offset += 1;
        }
        offset += 1;
    }


    // (todo) 01.1: Initialize VAO, and VBO
    vao.Bind();
    vbo.Bind();
    vbo.AllocateData(vertices.size() * (GL_FLOAT_VEC2 + GL_FLOAT_VEC3));
    vbo.UpdateData(std::span(vertices));

    VertexAttribute position(Data::Type::Float, 3);
    vao.SetAttribute(0, position, 0, sizeof(Vertex));

    // (todo) 01.5: Initialize EBO
    ebo.Bind();
    ebo.AllocateData<int>(std::span(indices));
    indicesSize = indices.size();

    // (todo) 01.1: Unbind VAO, and VBO
    VertexBufferObject::Unbind();
    VertexArrayObject::Unbind();

    // (todo) 01.5: Unbind EBO
    ElementBufferObject::Unbind();

}

void Particles::Update()
{
    Application::Update();

    auto p = vertices.begin();

    float threshold = 0.5f;

    vbo.Bind();
    vbo.UpdateData(std::span(vertices), 0);
    vao.Bind();


    UpdateOutputMode();
}

void Particles::Render()
{
    Application::Render();

    // Clear color and depth
    GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);

    // Set shader to be used
    glUseProgram(m_shaderProgram);

    // (todo) 01.1: Draw the grid
    vao.Bind();
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    //glDrawArrays(GL_TRIANGLES, 0, m_gridY * m_gridX * 6);
    glEnable(GL_DEPTH_TEST);
    glDrawElements(GL_POINTS, indicesSize, GL_UNSIGNED_INT, 0);

}

void Particles::Cleanup()
{
    Application::Cleanup();
}

void Particles::BuildShaders()
{
    const char* vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec2 aTexCoord;\n"
        "layout (location = 2) in vec3 aColor;\n"
        "layout (location = 3) in vec3 aNormal;\n"
        "uniform mat4 Matrix = mat4(1);\n"
        "out vec2 texCoord;\n"
        "out vec3 color;\n"
        "out vec3 normal;\n"
        "void main()\n"
        "{\n"
        "   texCoord = aTexCoord;\n"
        "   color = aColor;\n"
        "   normal = aNormal;\n"
        "   gl_Position = Matrix * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "}\0";
    const char* fragmentShaderSource = "#version 330 core\n"
        "uniform uint Mode = 0u;\n"
        "in vec2 texCoord;\n"
        "in vec3 color;\n"
        "in vec3 normal;\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "   switch (Mode)\n"
        "   {\n"
        "   default:\n"
        "   case 0u:\n"
        "       FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
        "       break;\n"
        "   case 1u:\n"
        "       FragColor = vec4(fract(texCoord), 0.0f, 1.0f);\n"
        "       break;\n"
        "   case 2u:\n"
        "       FragColor = vec4(color, 1.0f);\n"
        "       break;\n"
        "   case 3u:\n"
        "       FragColor = vec4(normalize(normal), 1.0f);\n"
        "       break;\n"
        "   case 4u:\n"
        "       FragColor = vec4(color * max(dot(normalize(normal), normalize(vec3(1,0,1))), 0.2f), 1.0f);\n"
        "       break;\n"
        "   }\n"
        "}\n\0";

    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    m_shaderProgram = shaderProgram;
}

void Particles::UpdateOutputMode()
{
    for (int i = 0; i <= 4; ++i)
    {
        if (GetMainWindow().IsKeyPressed(GLFW_KEY_0 + i))
        {
            int modeLocation = glGetUniformLocation(m_shaderProgram, "Mode");
            glUseProgram(m_shaderProgram);
            glUniform1ui(modeLocation, i);
            break;
        }
    }
    if (GetMainWindow().IsKeyPressed(GLFW_KEY_TAB))
    {
        const float projMatrix[16] = { 0, -1.294f, -0.721f, -0.707f, 1.83f, 0, 0, 0, 0, 1.294f, -0.721f, -0.707f, 0, 0, 1.24f, 1.414f };
        int matrixLocation = glGetUniformLocation(m_shaderProgram, "Matrix");
        glUseProgram(m_shaderProgram);
        glUniformMatrix4fv(matrixLocation, 1, false, projMatrix);
    }
}
