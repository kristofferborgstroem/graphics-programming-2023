#include "ViewerApplication.h"

#include <ituGL/asset/ShaderLoader.h>
#include <ituGL/asset/ModelLoader.h>
#include <ituGL/asset/Texture2DLoader.h>
#include <ituGL/shader/Material.h>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>
#include <imgui.h>
#include <iostream>

ViewerApplication::ViewerApplication()
    : Application(1024, 1024, "Viewer demo")
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

    InitializeModel();
    InitializeCamera();
    InitializeLights();

    DeviceGL& device = GetDevice();
    device.EnableFeature(GL_DEPTH_TEST);
    device.SetVSyncEnabled(true);
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

    // Clear color and depth
    GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);


    //ShadowPass();
    UpdateMaterials();
    m_model.Draw();
    RenderGUI();
}
/*
void ViewerApplication::ShadowPass() {
    /*
    unsigned int depthFBO;
    GetDevice().SetViewport(0, 0, m_shadowWidth, m_shadowHeight);
    glGenFramebuffers(1, &depthFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
    //glClear(GL_DEPTH_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    GetDevice().SetViewport(0, 0, 1024 * 2, 1024 * 2);
    m_camera.SetOrthographicProjectionMatrix({0.0f, 0.0f, 0.0f}, m_lightPosition);
}*/



void ViewerApplication::Cleanup()
{
    // Cleanup DearImGUI
    m_imGui.Cleanup();

    Application::Cleanup();
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
    filteredUniforms.insert("WorldMatrix");
    filteredUniforms.insert("ViewProjMatrix");
    filteredUniforms.insert("LightPosition");
    filteredUniforms.insert("LightColor");
    filteredUniforms.insert("AmbientColor");
    filteredUniforms.insert("CameraPosition");

    // Create reference material
    std::shared_ptr<Material> material = std::make_shared<Material>(shaderProgram, filteredUniforms);
    material->SetUniformValue("Color", glm::vec4(1.0f));

    // Setup function
    ShaderProgram::Location worldMatrixLocation = shaderProgram->GetUniformLocation("WorldMatrix");
    ShaderProgram::Location viewProjMatrixLocation = shaderProgram->GetUniformLocation("ViewProjMatrix");
    ShaderProgram::Location lightPositionLocation = shaderProgram->GetUniformLocation("LightPosition");
    ShaderProgram::Location lightColorLocation = shaderProgram->GetUniformLocation("LightColor");
    ShaderProgram::Location ambientColorLocation = shaderProgram->GetUniformLocation("AmbientColor");
    ShaderProgram::Location camPositionLocation = shaderProgram->GetUniformLocation("CameraPosition");
    material->SetShaderSetupFunction([=](ShaderProgram& shaderProgram)
        {
            shaderProgram.SetUniform(worldMatrixLocation, glm::scale(glm::vec3(0.1f)));
            shaderProgram.SetUniform(viewProjMatrixLocation, m_camera.GetViewProjectionMatrix());

            // (todo) 05.X: Set camera and light uniforms
            shaderProgram.SetUniform(lightPositionLocation, m_lightPosition);
            shaderProgram.SetUniform(lightColorLocation, m_lightColor * m_lightIntensity);
            shaderProgram.SetUniform(ambientColorLocation, m_ambientColor);
            shaderProgram.SetUniform(camPositionLocation, m_cameraPosition);

        });

    // Configure loader
    ModelLoader loader(material);
    loader.SetMaterialAttribute(VertexAttribute::Semantic::Position, "VertexPosition");
    loader.SetMaterialAttribute(VertexAttribute::Semantic::Normal, "VertexNormal");
    loader.SetMaterialAttribute(VertexAttribute::Semantic::TexCoord0, "VertexTexCoord");

    // Load model
    loader.SetCreateMaterials(true);
    m_model = loader.Load("models/mill/Mill.obj");

    // (todo) 05.1: Load and set textures
    Texture2DLoader texture2DLoader(TextureObject::FormatRGBA, TextureObject::InternalFormatRGBA8);
    texture2DLoader.SetFlipVertical(true);

    std::shared_ptr<Texture2DObject> shadow = std::make_shared<Texture2DObject>(texture2DLoader.Load("models/mill/Ground_shadow.jpg"));
    std::shared_ptr<Texture2DObject> ground = std::make_shared<Texture2DObject>(texture2DLoader.Load("models/mill/Ground_color.jpg"));
    std::shared_ptr<Texture2DObject> mill = std::make_shared<Texture2DObject>(texture2DLoader.Load("models/mill/MillCat_color.jpg"));

    m_model.GetMaterial(0).SetUniformValue("Albedo", shadow);
    m_model.GetMaterial(1).SetUniformValue("Albedo", ground);
    m_model.GetMaterial(2).SetUniformValue("Albedo", mill);
}

void ViewerApplication::InitializeCamera()
{
    // Set view matrix, from the camera position looking to the origin
    m_camera.SetViewMatrix(m_cameraPosition, glm::vec3(0.0f));

    // Set perspective matrix
    float aspectRatio = GetMainWindow().GetAspectRatio();
    m_camera.SetPerspectiveProjectionMatrix(1.0f, aspectRatio, 0.1f, 1000.0f);
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
