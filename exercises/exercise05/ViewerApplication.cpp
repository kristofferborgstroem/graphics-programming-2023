#include "ViewerApplication.h"

#include <ituGL/asset/ShaderLoader.h>
#include <ituGL/asset/ModelLoader.h>
#include <ituGL/asset/Texture2DLoader.h>
#include <ituGL/shader/Material.h>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>
#include <imgui.h>
#include <iostream>
#include <ituGL/geometry/VertexFormat.h>

ViewerApplication::ViewerApplication()
    : Application(800, 800, "Viewer demo")
    , m_cameraPosition(0, 30, 30)
    , m_cameraTranslationSpeed(20.0f)
    , m_cameraRotationSpeed(0.5f)
    , m_cameraEnabled(false)
    , m_cameraEnablePressed(false)
    , m_mousePosition(GetMainWindow().GetMousePosition(true))
{
}

void ViewerApplication::Initialize()
{
    Application::Initialize();

    // Initialize DearImGUI
    m_imGui.Initialize(GetMainWindow());


    m_quadMesh = std::make_shared<Mesh>();
    CreateQuadMesh(m_quadMesh);
    InitializeQuad();

    DeviceGL& device = GetDevice();
    device.EnableFeature(GL_DEPTH_TEST);
    device.EnableFeature(GL_CULL_FACE);
    device.SetVSyncEnabled(true);

    InitializeShadowTexture();
    InitializeShadows();
    InitializeModel();
    InitializeCamera();
    InitializeLights();
}

void ViewerApplication::Update()
{
    Application::Update();

    // Update camera controller
    UpdateCamera();
}

void ViewerApplication::UpdateMaterials() {
    m_model.GetMaterial(0).SetUniformValue("AmbientReflection", m_ambientReflectionShadow);
    m_model.GetMaterial(0).SetUniformValue("DiffuseReflection", m_diffuseReflectionShadow);
    m_model.GetMaterial(0).SetUniformValue("SpecularReflection", m_specularReflectionShadow);
    m_model.GetMaterial(0).SetUniformValue("SpecularExponent", m_specularExponentShadow);

    m_model.GetMaterial(1).SetUniformValue("AmbientReflection", m_ambientReflectionGround);
    m_model.GetMaterial(1).SetUniformValue("DiffuseReflection", m_diffuseReflectionGround);
    m_model.GetMaterial(1).SetUniformValue("SpecularReflection", m_specularReflectionGround);
    m_model.GetMaterial(1).SetUniformValue("SpecularExponent", m_specularExponentGround);

    m_model.GetMaterial(2).SetUniformValue("AmbientReflection", m_ambientReflectionMill);
    m_model.GetMaterial(2).SetUniformValue("DiffuseReflection", m_diffuseReflectionMill);
    m_model.GetMaterial(2).SetUniformValue("SpecularReflection", m_specularReflectionMill);
    m_model.GetMaterial(2).SetUniformValue("SpecularExponent", m_specularExponentMill);
}

void ViewerApplication::Render()
{
    Application::Render();

    GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);


    float nearPlane = m_nearPlane;
    float farPlane = m_farPlane;
    float w = 20.0f;
    glm::mat4 lightProjMatrix = glm::ortho(-w, w, -w, w, nearPlane, farPlane);
    glm::mat4 lightViewMatrix = glm::lookAt(m_lightPosition, glm::vec3(0.0f), {0.0f, 1.0f, 0.0f});
    m_lightSpaceMatrix = lightProjMatrix * lightViewMatrix;
    m_shadowModel.GetMaterial(0).SetUniformValue("LightSpaceMatrix", m_lightSpaceMatrix);
    m_shadowModel.GetMaterial(1).SetUniformValue("LightSpaceMatrix", m_lightSpaceMatrix);
    m_shadowModel.GetMaterial(2).SetUniformValue("LightSpaceMatrix", m_lightSpaceMatrix);

    ShadowPass();
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glActiveTexture(GL_TEXTURE5);
    m_quadModel.GetMaterial(0).GetShaderProgram()->Use();
    ShaderProgram::Location depthMapLocation = m_quadModel.GetMaterial(0).GetUniformLocation("depthMap");
    m_quadModel.GetMaterial(0).GetShaderProgram()->SetUniform(depthMapLocation, 5);
    m_quadModel.Draw();

    glBindTexture(GL_TEXTURE_2D, depthMap);
    glActiveTexture(GL_TEXTURE5);
    m_model.GetMaterial(2).GetShaderProgram()->Use();
    depthMapLocation = m_model.GetMaterial(2).GetUniformLocation("DepthMap");
    m_model.GetMaterial(2).GetShaderProgram()->SetUniform(depthMapLocation, 5);

    UpdateMaterials();
    m_model.Draw();

    RenderGUI();
}

void ViewerApplication::InitializeShadowTexture() {
    glGenFramebuffers(1, &depthMapFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_shadowWidth, m_shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    unsigned int depthrenderbuffer;
    glGenRenderbuffers(1, &depthrenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_shadowWidth, m_shadowHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "incomplete: " << glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
    }
}

void ViewerApplication::ShadowPass()
{
    GetDevice().SetViewport(0, 0,  m_shadowWidth, m_shadowHeight);
    glCullFace(GL_FRONT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);
    m_shadowModel.Draw();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    GetDevice().SetViewport(0, 0,  m_screenWidth, m_screenHeight);
    glCullFace(GL_BACK);
}

void ViewerApplication::Cleanup()
{
    // Cleanup DearImGUI
    m_imGui.Cleanup();

    Application::Cleanup();
}

void ViewerApplication::InitializeQuad() {
    Shader vertexShader = ShaderLoader::Load(Shader::VertexShader, "shaders/quad.vert");
    Shader fragmentShader = ShaderLoader::Load(Shader::FragmentShader, "shaders/quad.frag");
    std::shared_ptr<ShaderProgram> shaderProgram = std::make_shared<ShaderProgram>();
    shaderProgram->Build(vertexShader, fragmentShader);

    auto material = std::make_shared<Material>(shaderProgram);

    m_quadModel.SetMesh(m_quadMesh);
    m_quadModel.AddMaterial(material);
}

void ViewerApplication::InitializeShadows() {
    Shader vertexShader = ShaderLoader::Load(Shader::VertexShader, "shaders/shadow.vert");
    Shader fragmentShader = ShaderLoader::Load(Shader::FragmentShader, "shaders/shadow.frag");
    std::shared_ptr<ShaderProgram> shaderProgram = std::make_shared<ShaderProgram>();
    shaderProgram->Build(vertexShader, fragmentShader);

    m_shadowMaterial = std::make_shared<Material>(shaderProgram);
    //m_shadowMaterial->SetUniformValue("Model", glm::scale(glm::vec3(0.1f)));

    ModelLoader loader(m_shadowMaterial);
    loader.SetMaterialAttribute(VertexAttribute::Semantic::Position, "VertexPosition");
    loader.SetCreateMaterials(true);
    m_shadowModel = loader.Load("models/mill/Mill.obj");

    m_shadowModel.GetMaterial(0).SetUniformValue("Model", glm::scale(glm::vec3(0.0f)));
    m_shadowModel.GetMaterial(1).SetUniformValue("Model", glm::scale(glm::vec3(0.1f)));
    m_shadowModel.GetMaterial(2).SetUniformValue("Model", glm::scale(glm::vec3(0.1f)));
}

void ViewerApplication::InitializeModel()
{
    // Load and build shader
    Shader vertexShader = ShaderLoader::Load(Shader::VertexShader, "shaders/blinn-phong.vert");
    Shader fragmentShader = ShaderLoader::Load(Shader::FragmentShader, "shaders/blinn-phong.frag");
    std::shared_ptr<ShaderProgram> shaderProgram = std::make_shared<ShaderProgram>();
    shaderProgram->Build(vertexShader, fragmentShader);

    // Filter out uniforms that are not material properties
    ShaderUniformCollection::NameSet filteredUniforms;
    //filteredUniforms.insert("WorldMatrix");
    filteredUniforms.insert("ViewProjMatrix");
    filteredUniforms.insert("LightPosition");
    filteredUniforms.insert("LightColor");
    filteredUniforms.insert("AmbientColor");
    filteredUniforms.insert("CameraPosition");

    // Create reference material
    m_phongShader = std::make_shared<Material>(shaderProgram, filteredUniforms);
    m_phongShader->SetUniformValue("Color", glm::vec4(1.0f));

    // Setup function
    //ShaderProgram::Location worldMatrixLocation = shaderProgram->GetUniformLocation("WorldMatrix");
    ShaderProgram::Location viewProjMatrixLocation = shaderProgram->GetUniformLocation("ViewProjMatrix");
    ShaderProgram::Location lightSpaceMatrixLocation = shaderProgram->GetUniformLocation("LightSpaceMatrix");
    ShaderProgram::Location drawShadowLocation = shaderProgram->GetUniformLocation("DrawShadow");
    ShaderProgram::Location lightPositionLocation = shaderProgram->GetUniformLocation("LightPosition");
    ShaderProgram::Location lightColorLocation = shaderProgram->GetUniformLocation("LightColor");
    ShaderProgram::Location ambientColorLocation = shaderProgram->GetUniformLocation("AmbientColor");
    ShaderProgram::Location camPositionLocation = shaderProgram->GetUniformLocation("CameraPosition");
    m_phongShader->SetShaderSetupFunction([=](ShaderProgram& shaderProgram)
        {
            //shaderProgram.SetUniform(worldMatrixLocation, glm::scale(glm::vec3(0.1f)));
            shaderProgram.SetUniform(viewProjMatrixLocation, m_camera.GetViewProjectionMatrix());
            shaderProgram.SetUniform(lightSpaceMatrixLocation, m_lightSpaceMatrix);
            shaderProgram.SetUniform(drawShadowLocation, m_drawShadow);

            // (todo) 05.X: Set camera and light uniforms
            shaderProgram.SetUniform(lightPositionLocation, m_lightPosition);
            shaderProgram.SetUniform(lightColorLocation, m_lightColor * m_lightIntensity);
            shaderProgram.SetUniform(ambientColorLocation, m_ambientColor);
            shaderProgram.SetUniform(camPositionLocation, m_cameraPosition);

        });

    // Configure loader
    ModelLoader loader(m_phongShader);
    loader.SetMaterialAttribute(VertexAttribute::Semantic::Position, "VertexPosition");
    loader.SetMaterialAttribute(VertexAttribute::Semantic::Normal, "VertexNormal");
    loader.SetMaterialAttribute(VertexAttribute::Semantic::TexCoord0, "VertexTexCoord");

    // Load model
    loader.SetCreateMaterials(true);
    m_model = loader.Load("models/mill/Mill.obj");

    // (todo) 05.1: Load and set textures
    Texture2DLoader texture2DLoader(TextureObject::FormatRGBA, TextureObject::InternalFormatRGBA8);
    texture2DLoader.SetFlipVertical(true);

    m_shadowTexture = std::make_shared<Texture2DObject>(texture2DLoader.Load("models/mill/Ground_shadow.jpg"));
    m_groundTexture = std::make_shared<Texture2DObject>(texture2DLoader.Load("models/mill/Ground_color.jpg"));
    m_millTexture = std::make_shared<Texture2DObject>(texture2DLoader.Load("models/mill/MillCat_color.jpg"));

    m_model.GetMaterial(0).SetUniformValue("Albedo", m_shadowTexture);
    m_model.GetMaterial(1).SetUniformValue("Albedo", m_groundTexture);
    m_model.GetMaterial(2).SetUniformValue("Albedo", m_millTexture);

    m_model.GetMaterial(0).SetUniformValue("WorldMatrix", glm::scale(glm::vec3(1.0f)));
    m_model.GetMaterial(1).SetUniformValue("WorldMatrix", glm::scale(glm::vec3(0.1f)));
    m_model.GetMaterial(2).SetUniformValue("WorldMatrix", glm::scale(glm::vec3(0.1f)));
}

void ViewerApplication::InitializeCamera()
{
    // Set view matrix, from the camera position looking to the origin
    m_camera.SetViewMatrix(m_cameraPosition, glm::vec3(0.0f));

    // Set perspective matrix
    float aspectRatio = GetMainWindow().GetAspectRatio();
    m_camera.SetPerspectiveProjectionMatrix(1.0f, aspectRatio, 0.1f, 100.0f);
}

void ViewerApplication::InitializeLights()
{
    // (todo) 05.X: Initialize light variables

}

void ViewerApplication::RenderGUI()
{
    m_imGui.BeginFrame();
    ImGui::ColorEdit3("AmbientColor", &m_ambientColor.x);
    ImGui::Separator();
    ImGui::DragFloat("LightIntensity", &m_lightIntensity, 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat3("LightPosition", &m_lightPosition.x);
    ImGui::ColorEdit3("LightColor", &m_lightColor.x);
    ImGui::Separator();
    ImGui::Text("DepthShader");
    ImGui::DragFloat("nearPlane", &m_nearPlane, 0.1f, 0.0f, 10.0f);
    ImGui::DragFloat("farPlane", &m_farPlane, 1.0f, 2.0f, 1000.0f);
    ImGui::DragFloat("drawShadow", &m_drawShadow, 0.001f, 0.0f, 3.0f);
    ImGui::Separator();
    ImGui::Text("Mill");
    ImGui::DragFloat("AmbientReflectionMill", &m_ambientReflectionMill, 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat("DiffuseReflectionMill", &m_diffuseReflectionMill, 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat("SpecularReflectionMill", &m_specularReflectionMill, 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat("SpecularExponentMill", &m_specularExponentMill, 1.0f, -1.0f, 1000.0f);
    ImGui::Separator();
    ImGui::Text("Ground");
    ImGui::DragFloat("AmbientReflectionGround", &m_ambientReflectionGround, 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat("DiffuseReflectionGround", &m_diffuseReflectionGround, 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat("SpecularReflectionGround", &m_specularReflectionGround, 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat("SpecularExponentGround", &m_specularExponentGround, 1.0f, -1.0f, 1000.0f);
    ImGui::Separator();
    ImGui::Text("Shadow");
    ImGui::DragFloat("AmbientReflectionShadow", &m_ambientReflectionShadow, 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat("DiffuseReflectionShadow", &m_diffuseReflectionShadow, 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat("SpecularReflectionShadow", &m_specularReflectionShadow, 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat("SpecularExponentShadow", &m_specularExponentShadow, 1.0f, -1.0f, 1000.0f);

    // (todo) 05.4: Add debug controls for light properties

    m_imGui.EndFrame();
}

void ViewerApplication::CreateQuadMesh(std::shared_ptr<Mesh> mesh) {

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

    vertices.emplace_back(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f));
    vertices.emplace_back(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f));
    vertices.emplace_back(glm::vec3(1.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f));
    vertices.emplace_back(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f));
    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(0);
    indices.push_back(2);
    indices.push_back(3);

    mesh->AddSubmesh<Vertex, unsigned int, VertexFormat::LayoutIterator>(Drawcall::Primitive::Triangles, vertices, indices,
             vertexFormat.LayoutBegin(static_cast<int>(vertices.size()), true), vertexFormat.LayoutEnd());
}

void ViewerApplication::UpdateCamera()
{
    Window& window = GetMainWindow();

    // Update if camera is enabled (controlled by SPACE key)
    {
        bool enablePressed = window.IsKeyPressed(GLFW_KEY_SPACE);
        if (enablePressed && !m_cameraEnablePressed)
        {
            m_cameraEnabled = !m_cameraEnabled;

            window.SetMouseVisible(!m_cameraEnabled);
            m_mousePosition = window.GetMousePosition(true);
        }
        m_cameraEnablePressed = enablePressed;
    }

    if (!m_cameraEnabled)
        return;

    glm::mat4 viewTransposedMatrix = glm::transpose(m_camera.GetViewMatrix());
    glm::vec3 viewRight = viewTransposedMatrix[0];
    glm::vec3 viewForward = -viewTransposedMatrix[2];

    // Update camera translation
    {
        glm::vec2 inputTranslation(0.0f);

        if (window.IsKeyPressed(GLFW_KEY_A))
            inputTranslation.x = -1.0f;
        else if (window.IsKeyPressed(GLFW_KEY_D))
            inputTranslation.x = 1.0f;

        if (window.IsKeyPressed(GLFW_KEY_W))
            inputTranslation.y = 1.0f;
        else if (window.IsKeyPressed(GLFW_KEY_S))
            inputTranslation.y = -1.0f;

        inputTranslation *= m_cameraTranslationSpeed;
        inputTranslation *= GetDeltaTime();

        // Double speed if SHIFT is pressed
        if (window.IsKeyPressed(GLFW_KEY_LEFT_SHIFT))
            inputTranslation *= 2.0f;

        m_cameraPosition += inputTranslation.x * viewRight + inputTranslation.y * viewForward;
    }

    // Update camera rotation
   {
        glm::vec2 mousePosition = window.GetMousePosition(true);
        glm::vec2 deltaMousePosition = mousePosition - m_mousePosition;
        m_mousePosition = mousePosition;

        glm::vec3 inputRotation(-deltaMousePosition.x, deltaMousePosition.y, 0.0f);

        inputRotation *= m_cameraRotationSpeed;

        viewForward = glm::rotate(inputRotation.x, glm::vec3(0,1,0)) * glm::rotate(inputRotation.y, glm::vec3(viewRight)) * glm::vec4(viewForward, 0);
    }

   // Update view matrix
   m_camera.SetViewMatrix(m_cameraPosition, m_cameraPosition + viewForward);
}
