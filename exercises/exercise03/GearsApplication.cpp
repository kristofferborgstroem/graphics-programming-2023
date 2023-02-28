#include "GearsApplication.h"

#include <ituGL/shader/Shader.h>
#include <ituGL/shader/ShaderProgram.h>
#include <ituGL/geometry/VertexFormat.h>
#include <cassert>  // for asserts
#include <array>    // to get shader error messages
#include <fstream>  // shader loading
#include <sstream>  // shader loading
#include <iostream> // to print messages to the console
#include <vector>   // to store vertices and indices
#include <numbers>  // for PI constant
#include <glm/gtx/transform.hpp>  // for matrix transformations

GearsApplication::GearsApplication()
    : Application(800, 800, "Gears demo")
    , m_colorUniform(-1)
{
}

void GearsApplication::Initialize()
{
    Application::Initialize();

    InitializeGeometry();

    InitializeShaders();

    GetDevice().EnableFeature(GL_DEPTH_TEST);
}

void GearsApplication::Update()
{
    Application::Update();

    const Window& window = GetMainWindow();

    // (todo) 03.5: Update the camera matrices
    float t = GetCurrentTime();
    float d = 2.0f * (glm::sin(t) + 2.0f);
    float x = glm::sin(t) * d;
    float z = glm::cos(t) * d;
    int width, height;
    window.GetDimensions(width, height);

    camera.SetViewMatrix({x, 0.0f, z}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
    camera.SetPerspectiveProjectionMatrix(1.0f, (float) width / (float) height, 0.1f, 10.0f);


}

void GearsApplication::Render()
{
    // Clear background
    GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f), true, 1.0f);

    // Set our shader program
    m_shaderProgram.Use();

    // (todo) 03.5: Set the view projection matrix from the camera. Once set, we will use it for all the objects
    m_shaderProgram.SetUniform(m_projUniform, camera.GetViewProjectionMatrix());


    // (todo) 03.1: Draw large gear at the center

    float speed = 4.0f;


    auto rotate = glm::rotate(glm::mat4(1.0f),
                              glm::radians(GetCurrentTime() * -20.0f * speed), {0.0f, 0.0f, 1.0f});
    glm::mat4 centerGearMatrix(1.0f);
    DrawGear(m_largeGear, centerGearMatrix * rotate, Color(0.6f, 0.4f, 0.3f));

    // (todo) 03.2: Draw medium gear to the right

    rotate = glm::rotate(glm::mat4(1.0f),
                         glm::radians(GetCurrentTime() * 40.0f * speed), {0.0f, 0.0f, 1.0f});

    auto mediumGear = glm::translate(glm::mat4(1.0f), {0.75f, 0.0f, 0.0f});
    DrawGear(m_mediumGear, mediumGear * rotate, Color(0.3f, 0.8f, 0.5f));


    // (todo) 03.3: Draw small gear at the top-left corner

    auto smallGear = glm::translate(glm::mat4(1.0f), {-1.0f, 1.0f, 0.0});
    auto smallRotate = glm::rotate(glm::radians(GetCurrentTime() * 10.66f * speed + 9.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    auto scale = glm::scale(glm::vec3(7.5f, 7.5f, 7.5f));

    DrawGear(m_smallGear, smallGear * smallRotate * scale, Color(0.1f, 0.8f, 0.2f));


    // (todo) 03.4: Draw small gear linked to the center gear

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    Application::Render();
}

// Create the meshes that we will use during the exercise
void GearsApplication::InitializeGeometry()
{
    CreateGearMesh(m_largeGear, 16, 0.2f, 0.5f, 0.1f, 0.5f, 0.1f);
    CreateGearMesh(m_mediumGear, 8, 0.05f, 0.25f, 0.1f, 0.5f, 0.3f);
    CreateGearMesh(m_smallGear, 30, 0.05f, 0.121f, 0.0125f, 0.5f, 0.015f);
}

// Load, compile and Build shaders
void GearsApplication::InitializeShaders()
{
    // Load and compile vertex shader
    Shader vertexShader(Shader::VertexShader);
    LoadAndCompileShader(vertexShader, "shaders/basic.vert");

    // Load and compile fragment shader
    Shader fragmentShader(Shader::FragmentShader);
    LoadAndCompileShader(fragmentShader, "shaders/basic.frag");

    // Attach shaders and link
    if (!m_shaderProgram.Build(vertexShader, fragmentShader))
    {
        std::cout << "Error linking shaders" << std::endl;
        return;
    }

    m_colorUniform = m_shaderProgram.GetUniformLocation("Color");

    // (todo) 03.1: Find the WorldMatrix uniform location
    m_modelUniform = m_shaderProgram.GetUniformLocation("Model");


    // (todo) 03.5: Find the ViewProjMatrix uniform location
    m_projUniform = m_shaderProgram.GetUniformLocation("ViewProjMatrix");


}

// Draw a gear mesh with a specific world matrix and color
void GearsApplication::DrawGear(const Mesh& mesh, const glm::mat4& worldMatrix, const Color& color)
{
    m_shaderProgram.SetUniform(m_colorUniform, static_cast<glm::vec3>(color));

    // (todo) 03.1: Set the value of the WorldMatrix uniform
    m_shaderProgram.SetUniform(m_modelUniform, worldMatrix);


    mesh.DrawSubmesh(0);
}

void GearsApplication::CreateGearMesh(Mesh& mesh, unsigned int cogCount, float innerRadius, float pitchRadius, float addendum, float cogRatio, float depth)
{
    /*
    cogCount = number of teeth
    innerRadius = radius of the inner circle
    pitchRadius = radius until middle of teeth.
    addendum = length of teeth
    cogRatio = ???
    depth = thickness of gear?
     */
    // Define the vertex structure
    struct Vertex
    {
        Vertex() = default;
        Vertex(const glm::vec3& position, const glm::vec3& normal) : position(position), normal(normal) {}
        glm::vec3 position;
        glm::vec3 normal;
    };

    // Define the vertex format (should match the vertex structure)
    VertexFormat vertexFormat;
    vertexFormat.AddVertexAttribute<float>(3);
    vertexFormat.AddVertexAttribute<float>(3);

    // List of vertices (VBO)
    std::vector<Vertex> vertices;

    // List of indices (EBO)
    std::vector<unsigned short> indices;

    // Circle
    unsigned int sides = 2 * cogCount;
    float deltaAngle = 360.0f / (float) sides;

    // Circle vertices
    float outerRadius = pitchRadius - 0.5f * addendum;
    for (int i = 0; i < sides; ++i) {

        bool isCog = i % 2 == 0;

        if (isCog) {

        }
        float radians = glm::radians((float) i * deltaAngle);
        glm::vec3 outerVertex = {glm::sin(radians) * outerRadius, glm::cos(radians) * outerRadius, 0.0f};
        glm::vec3 innerVertex = {glm::sin(radians) * innerRadius, glm::cos(radians) * innerRadius, 0.0f};

        // Front
        vertices.emplace_back(outerVertex, glm::vec3(0.0f, 0.0f, -1.0f));
        vertices.emplace_back(innerVertex, glm::vec3(0.0f, 0.0f, -1.0f));

        // Back
        vertices.emplace_back(glm::vec3(outerVertex.x, outerVertex.y, depth), glm::vec3(0.0f, 0.0f, 1.0f));
        vertices.emplace_back(glm::vec3(innerVertex.x, innerVertex.y, depth), glm::vec3(0.0f, 0.0f, 1.0f));
    }

    // Circle indices - ccw
    // 2 for each face, 2 for front and back.

    unsigned int triangles = 4 * sides;
    for (int i = 0; i < sides; ++i) {
        // Front
        indices.push_back(i * 4 + 1);                     // Bottom Left - Front
        indices.push_back(((i + 1) * 4) % triangles);     // Top Right - Front
        indices.push_back(i * 4);                         // Top Left - Front

        indices.push_back(i * 4 + 1);                     // Bottom Left - Front
        indices.push_back(((i + 1) * 4 + 1) % triangles); // Bottom Right - Front
        indices.push_back(((i + 1) * 4) % triangles);     // Top Right - Front

        // Back
        indices.push_back(i * 4 + 3);                     // Bottom Left - Back
        indices.push_back(i * 4 + 2);                     // Top Left - Back
        indices.push_back(((i + 1) * 4 + 2) % triangles); // Top Right - Back

        indices.push_back(((i + 1) * 4 + 3) % triangles); // Bottom Right - Back
        indices.push_back(i * 4 + 3);                     // Bottom Left - Back
        indices.push_back(((i + 1) * 4 + 2) % triangles); // Top Right - Back

        // Inner fan
        indices.push_back(i * 4 + 1);                     // Bottom Left - Front
        indices.push_back(((i + 1) * 4 + 3) % triangles); // Bottom Right - Back
        indices.push_back(i * 4 + 3);                     // Bottom Left - Back

        indices.push_back(i * 4 + 1);                     // Bottom Left - Front
        indices.push_back(((i + 1) * 4 + 1) % triangles); // Bottom Right - Front
        indices.push_back(((i + 1) * 4 + 3) % triangles); // Bottom Right - Back

        // Outer fan
        indices.push_back(i * 4);                         // Top Left - Front
        indices.push_back(((i + 1) * 4 + 2) % triangles); // Top Right - Back
        indices.push_back(i * 4 + 2);                     // Top Left - Back

        indices.push_back(i * 4);                         // Top Left - Front
        indices.push_back(((i + 1) * 4) % triangles);     // Top Right - Front
        indices.push_back(((i + 1) * 4 + 2) % triangles); // Top Right - Back
    }


    std::vector<Vertex> cogVertices;
    std::vector<unsigned int> cogIndices;
    // Teeth
    innerRadius = outerRadius - 0.5f * addendum;
    outerRadius = pitchRadius + 0.5f * addendum;
    //deltaAngle = 360.0f / (float) cogCount;

    for (int i = 0; i < cogCount; ++i) {

        // cogRation = 0.5;
        float start  = glm::radians((float) i * 2 * deltaAngle);
        float end    = glm::radians(((float) i * 2 + 1.0f) * deltaAngle);
        float middle = glm::radians(deltaAngle * 0.5f) + start;
        float deltaCogRatio = glm::radians(deltaAngle * cogRatio * 0.5f);
        float z = depth * cogRatio * 0.5f;

        auto topLeft = glm::vec3(glm::sin(middle - deltaCogRatio) * outerRadius, glm::cos(middle - deltaCogRatio) * outerRadius, z);
        auto topRight = glm::vec3(glm::sin(middle + deltaCogRatio) * outerRadius, glm::cos(middle + deltaCogRatio) * outerRadius, z);
        deltaCogRatio = glm::radians(deltaAngle * (0.5f - cogRatio * 0.5f));
        auto bottomLeft = glm::vec3(glm::sin(start - deltaCogRatio) * innerRadius, glm::cos(start - deltaCogRatio) * innerRadius, 0.0f);
        auto bottomRight = glm::vec3(glm::sin(end + deltaCogRatio) * innerRadius, glm::cos(end + deltaCogRatio) * innerRadius, 0.0f);

        //auto topRight = glm::vec3(glm::sin(middle + deltaCogRatio) * outerRadius, glm::cos(middle + deltaCogRatio) * outerRadius, 0.0f);

        // Front

        vertices.emplace_back(topLeft, glm::vec3(0.0f, 0.0f, -1.0f));
        vertices.emplace_back(bottomLeft, glm::vec3(0.0f, 0.0f, -1.0f));
        vertices.emplace_back(topRight, glm::vec3(0.0f, 0.0f, -1.0f));
        vertices.emplace_back(bottomRight, glm::vec(glm::vec3(0.0f, 0.0f, -1.0f)));

        // Back
        auto topLeftBack = glm::vec3(topLeft.x, topLeft.y, depth - z);
        auto topRightBack = glm::vec3(topRight.x, topRight.y, depth - z);

        vertices.emplace_back(topLeftBack, glm::vec3(0.0f, 0.0f, 1.0f));
        vertices.emplace_back(glm::vec3(bottomLeft.x, bottomLeft.y, depth), glm::vec3(0.0f, 0.0f, 1.0f));
        vertices.emplace_back(topRightBack, glm::vec3(0.0f, 0.0f, 1.0f));
        vertices.emplace_back(glm::vec3(bottomRight.x, bottomRight.y, depth), glm::vec3(0.0f, 0.0f, 1.0f));

    }

    unsigned int totalTriangles = triangles + 8 * cogCount;
    for (int i = 0; i < cogCount; ++i) {
        // Front
        indices.push_back((triangles + i * 8 + 1) % totalTriangles);     // Bottom Left - Front
        indices.push_back((triangles + i * 8 + 2) % totalTriangles);     // Top Right - Front
        indices.push_back((triangles + i * 8 + 0) % totalTriangles);     // Top Left - Front

        indices.push_back((triangles + i * 8 + 1) % totalTriangles);     // Bottom Left - Front
        indices.push_back((triangles + i * 8 + 3) % totalTriangles);     // Bottom Right - Front
        indices.push_back((triangles + i * 8 + 2) % totalTriangles);     // Top Right - Front

        // Back
        indices.push_back((triangles + i * 8 + 5) % totalTriangles);     // Bottom Left - Back
        indices.push_back((triangles + i * 8 + 6) % totalTriangles);     // Top Right - Back
        indices.push_back((triangles + i * 8 + 4) % totalTriangles);     // Top Left - Back

        indices.push_back((triangles + i * 8 + 5) % totalTriangles);     // Bottom Left - Back
        indices.push_back((triangles + i * 8 + 7) % totalTriangles);     // Bottom Right - Back
        indices.push_back((triangles + i * 8 + 6) % totalTriangles);     // Top Right - Back

        // Sides
        // Left
        indices.push_back((triangles + i * 8 + 1) % totalTriangles);     // Bottom Left - Front
        indices.push_back((triangles + i * 8 + 4) % totalTriangles);     // Top Left - Back
        indices.push_back((triangles + i * 8 + 5) % totalTriangles);     // Bottom Left - Back

        indices.push_back((triangles + i * 8 + 1) % totalTriangles);     // Bottom Left - Front
        indices.push_back((triangles + i * 8 + 0) % totalTriangles);     // Top Left - Front
        indices.push_back((triangles + i * 8 + 4) % totalTriangles);     // Top Left - Back

        // Right
        indices.push_back((triangles + i * 8 + 3) % totalTriangles);     // Bottom Right - Front
        indices.push_back((triangles + i * 8 + 7) % totalTriangles);     // Bottom Right - Back
        indices.push_back((triangles + i * 8 + 6) % totalTriangles);     // Top Right - Back

        indices.push_back((triangles + i * 8 + 3) % totalTriangles);     // Bottom Right - Front
        indices.push_back((triangles + i * 8 + 6) % totalTriangles);     // Top Right - Back
        indices.push_back((triangles + i * 8 + 2) % totalTriangles);     // Top Right - Front

        // Top
        indices.push_back((triangles + i * 8 + 0) % totalTriangles);     // Top Left - Front
        indices.push_back((triangles + i * 8 + 2) % totalTriangles);     // Top Right - Front
        indices.push_back((triangles + i * 8 + 6) % totalTriangles);     // Top Right - Back

        indices.push_back((triangles + i * 8 + 0) % totalTriangles);     // Top Left - Front
        indices.push_back((triangles + i * 8 + 6) % totalTriangles);     // Top Right - Back
        indices.push_back((triangles + i * 8 + 4) % totalTriangles);     // Top Left - Back
    }

    // Finally create the new submesh with all the data
    mesh.AddSubmesh<Vertex, unsigned short, VertexFormat::LayoutIterator>(Drawcall::Primitive::Triangles, vertices, indices,
        vertexFormat.LayoutBegin(static_cast<int>(vertices.size()), true /* interleaved */), vertexFormat.LayoutEnd());


}

void GearsApplication::LoadAndCompileShader(Shader& shader, const char* path)
{
    // Open the file for reading
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cout << "Can't find file: " << path << std::endl;
        std::cout << "Is your working directory properly set?" << std::endl;
        return;
    }

    // Dump the contents into a string
    std::stringstream stringStream;
    stringStream << file.rdbuf() << '\0';

    // Set the source code from the string
    shader.SetSource(stringStream.str().c_str());

    // Try to compile
    if (!shader.Compile())
    {
        // Get errors in case of failure
        std::array<char, 256> errors;
        shader.GetCompilationErrors(errors);
        std::cout << "Error compiling shader: " << path << std::endl;
        std::cout << errors.data() << std::endl;
    }
}
