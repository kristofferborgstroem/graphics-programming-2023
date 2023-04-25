//
// Created by Kristoffer Borgstrøm on 21/03/2023.
//
#include <iostream>
#include <memory>

#include "CloudApplication.hpp"
#include "Cloud.hpp"
#include "Cube.hpp"

#include <ituGL/asset/ModelLoader.h>
#include <ituGL/asset/ShaderLoader.h>
#include <ituGL/asset/Texture2DLoader.h>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>
#include <imgui.h>

CloudApplication::CloudApplication() :
        Application(1024, 1024, "Cloud Application"),
        m_cloud(64, 32)
{
}

void CloudApplication::Initialize() {
    Application::Initialize();

    m_imGui.Initialize(GetMainWindow());
    controller.Initialize(GetMainWindow());

    DeviceGL& device = GetDevice();
    device.EnableFeature(GL_DEPTH_TEST);
    device.EnableFeature(GL_CULL_FACE);
    device.SetVSyncEnabled(true);

    m_cloud.InitializeCloud();
    m_cloudNoise = m_cloud.GetHighResCloud();
    m_cloudNoiseLowRes = m_cloud.GetLowResCloud();
    m_cloudMesh = Cube::InitializeQuad();
    InitializeShaders();

    InitModel();
    InitCube();
}

void CloudApplication::Update() {
    Application::Update();

    controller.UpdateCamera(GetMainWindow(), GetDeltaTime());
}

void CloudApplication::Render() {
    Application::Render();

    GetDevice().Clear(true, Color(0.4f, 0.4f, 0.9f, 1.0f), true, 1.0f);

    DeviceGL& device = GetDevice();
    device.SetCullFace(GL_BACK);
    m_model.Draw();

    device.SetCullFace(GL_FRONT);
    m_sunModel.Draw();
    m_cloudModel.Draw();
    RenderGUI();
}

void CloudApplication::RenderGUI() {
    m_imGui.BeginFrame();
    ImGui::DragFloat("LightIntensity", &m_cloudMarchAbsorption, 0.01f, 0.0f, 10.0f);
    ImGui::DragFloat("AnimationSpeed", &m_animationSpeed, 0.1f, 0.0f, 32.0f);
    ImGui::DragFloat3("LightPosition", &m_lightPosition.x);
    ImGui::DragFloat("Absorption", &m_lightMarchAbsorption, 0.01f, 0.0f, 10.0f);
    ImGui::DragInt("LightSteps", &m_lightSteps, 1.0f, 1.0f, 50.0f);
    ImGui::DragInt("CloudSteps", &m_cloudSteps, 1.0f, 1.0f, 50.0f);
    ImGui::DragFloat("ForwardScattering", &m_forwardScattering, 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat("BackwardScattering", &m_backwardScattering, 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat("Thickness", &m_thickness, 0.01f, 0.0f, 2.0f);
    ImGui::DragFloat("Scale", &m_scale, 0.1f, 0.0f, 10.0f);
    ImGui::DragFloat("WatherScale", &m_weatherScale, 0.1f, 0.0f, 10.0f);
    ImGui::DragInt("Debug", &m_debug, 1.0f, 0.0f, 3.0f);
    ImGui::Separator();
    ImGui::DragFloat3("CloudSampling", &m_cloudSampling.x, 0.01f, 0.0f, 1.0f);

    m_imGui.EndFrame();
}

void CloudApplication::InitializeShaders()
{
    InitializeCloudShader();
    InitializeUnlitShader();
    InitializeSunShader();
}

void CloudApplication::InitializeCloudShader()
{
    Shader vertexShader = ShaderLoader::Load(Shader::VertexShader, "shaders/cloud.vert");
    Shader fragmentShader = ShaderLoader::Load(Shader::FragmentShader, "shaders/cloud.frag");
    std::shared_ptr<ShaderProgram> shaderProgram = std::make_shared<ShaderProgram>();
    shaderProgram->Build(vertexShader, fragmentShader);

    ShaderUniformCollection::NameSet filteredUniforms;
    filteredUniforms.insert("Slice");
    filteredUniforms.insert("Time");
    filteredUniforms.insert("ViewProjectionMatrix");
    filteredUniforms.insert("LocalCameraPosition");
    filteredUniforms.insert("LightSteps");
    filteredUniforms.insert("CloudSteps");

    m_cloudMaterial = std::make_shared<Material>(shaderProgram, filteredUniforms);
    m_cloudMaterial->SetUniformValue("Noise", m_cloudNoise);
    m_cloudMaterial->SetUniformValue("LowResNoise", m_cloudNoiseLowRes);
    m_cloudMaterial->SetUniformValue("WorldMatrix", glm::scale(glm::vec3(0.1f)));

    Texture2DLoader loader(TextureObject::FormatRGBA, TextureObject::InternalFormatRGBA8);
    auto weathermap = std::make_shared<Texture2DObject>(loader.Load("models/cloud/weathermap.png"));
    weathermap->Bind();
    weathermap->GenerateMipmap();
    weathermap->SetParameter(Texture2DObject::ParameterEnum::WrapS, GL_MIRRORED_REPEAT);
    weathermap->SetParameter(Texture2DObject::ParameterEnum::WrapT, GL_MIRRORED_REPEAT);

    m_cloudMaterial->SetUniformValue("WeatherMap", weathermap);

    ShaderProgram::Location viewProjectionLocation = shaderProgram->GetUniformLocation("ViewProjectionMatrix");
    ShaderProgram::Location cloudAbsorptionLocation = shaderProgram->GetUniformLocation("CloudAbsorption");
    ShaderProgram::Location lightAbsorptionLocation = shaderProgram->GetUniformLocation("LightAbsorption");
    ShaderProgram::Location timeLocation = shaderProgram->GetUniformLocation("Time");
    ShaderProgram::Location animationLocation = shaderProgram->GetUniformLocation("AnimationSpeed");
    ShaderProgram::Location localCameraLocation = shaderProgram->GetUniformLocation("LocalCameraPosition");
    ShaderProgram::Location lightLocation = shaderProgram->GetUniformLocation("LightPosition");
    ShaderProgram::Location lightStepsLocation = shaderProgram->GetUniformLocation("LightSteps");
    ShaderProgram::Location cloudStepsLocation = shaderProgram->GetUniformLocation("CloudSteps");
    ShaderProgram::Location forwardLocation = shaderProgram->GetUniformLocation("ForwardScattering");
    ShaderProgram::Location backLocation = shaderProgram->GetUniformLocation("BackwardScattering");
    ShaderProgram::Location thicknessLocation = shaderProgram->GetUniformLocation("Thickness");
    ShaderProgram::Location cloudSamplingLocation = shaderProgram->GetUniformLocation("CloudSampling");
    ShaderProgram::Location scaleLocation = shaderProgram->GetUniformLocation("Scale");
    ShaderProgram::Location weatherScaleLocation = shaderProgram->GetUniformLocation("WeatherScale");
    ShaderProgram::Location debugLocation = shaderProgram->GetUniformLocation("Debug");

    m_cloudMaterial->SetShaderSetupFunction([=](ShaderProgram& shaderProgram) {
        shaderProgram.SetUniform(viewProjectionLocation, controller.GetViewProjectionMatrix());
        shaderProgram.SetUniform(cloudAbsorptionLocation, m_cloudMarchAbsorption);
        shaderProgram.SetUniform(lightAbsorptionLocation, m_lightMarchAbsorption);
        shaderProgram.SetUniform(timeLocation, GetCurrentTime() / 4.0f);
        shaderProgram.SetUniform(animationLocation, m_animationSpeed);
        shaderProgram.SetUniform(cloudStepsLocation, m_cloudSteps);
        shaderProgram.SetUniform(lightStepsLocation, m_lightSteps);
        shaderProgram.SetUniform(forwardLocation, m_forwardScattering);
        shaderProgram.SetUniform(backLocation, m_backwardScattering);
        shaderProgram.SetUniform(thicknessLocation, m_thickness);
        shaderProgram.SetUniform(cloudSamplingLocation, m_cloudSampling);
        shaderProgram.SetUniform(scaleLocation, m_scale);
        shaderProgram.SetUniform(weatherScaleLocation, m_weatherScale);
        shaderProgram.SetUniform(debugLocation, m_debug);

        shaderProgram.SetUniform(localCameraLocation, (glm::inverse(m_worldMatrix) * glm::vec4(controller.GetCameraPosition(), 1.0f)));

        auto lightPosition = glm::inverse(m_worldMatrix) * glm::vec4(m_lightPosition, 1.0);
        shaderProgram.SetUniform(lightLocation, glm::vec3(lightPosition.x, lightPosition.y, lightPosition.z));

    });

    m_cloudMaterial->SetBlendEquation(Material::BlendEquation::Add);
    m_cloudMaterial->SetBlendParams(Material::BlendParam::SourceAlpha, Material::BlendParam::OneMinusSourceAlpha);
}

void CloudApplication::InitializeUnlitShader()
{
    Shader vertexShader = ShaderLoader::Load(Shader::VertexShader, "shaders/unlit.vert");
    Shader fragmentShader = ShaderLoader::Load(Shader::FragmentShader, "shaders/unlit.frag");
    std::shared_ptr<ShaderProgram> shaderProgram = std::make_shared<ShaderProgram>();
    shaderProgram->Build(vertexShader, fragmentShader);

    ShaderUniformCollection::NameSet filteredUniforms;
    filteredUniforms.insert("ViewProjectionMatrix");

    m_unlitMaterial = std::make_shared<Material>(shaderProgram, filteredUniforms);
    m_unlitMaterial->SetUniformValue("WorldMatrix", glm::scale(glm::vec3(0.1f)));

    ShaderProgram::Location viewProjectionLocation = shaderProgram->GetUniformLocation("ViewProjectionMatrix");
    m_unlitMaterial->SetShaderSetupFunction([=](ShaderProgram& shaderProgram) {
        shaderProgram.SetUniform(viewProjectionLocation, controller.GetViewProjectionMatrix());
    });
}

void CloudApplication::InitializeSunShader()
{
    Shader vertexShader = ShaderLoader::Load(Shader::VertexShader, "shaders/sun.vert");
    Shader fragmentShader = ShaderLoader::Load(Shader::FragmentShader, "shaders/sun.frag");
    std::shared_ptr<ShaderProgram> shaderProgram = std::make_shared<ShaderProgram>();
    shaderProgram->Build(vertexShader, fragmentShader);

    ShaderUniformCollection::NameSet filteredUniforms;
    filteredUniforms.insert("WorldMatrix");
    filteredUniforms.insert("ViewProjectionMatrix");

    m_sunMaterial = std::make_shared<Material>(shaderProgram, filteredUniforms);

    ShaderProgram::Location viewProjectionLocation = shaderProgram->GetUniformLocation("ViewProjectionMatrix");
    ShaderProgram::Location worldMatrixLocation = shaderProgram->GetUniformLocation("WorldMatrix");
    m_sunMaterial->SetShaderSetupFunction([=](ShaderProgram& shaderProgram) {
        shaderProgram.SetUniform(worldMatrixLocation, glm::translate(m_lightPosition));
        shaderProgram.SetUniform(viewProjectionLocation, controller.GetViewProjectionMatrix());
    });
}

void CloudApplication::InitModel() {
    ModelLoader loader(m_unlitMaterial);
    loader.SetMaterialAttribute(VertexAttribute::Semantic::Position, "VertexPosition");
    loader.SetMaterialAttribute(VertexAttribute::Semantic::TexCoord0, "TextureCoord");
    loader.SetCreateMaterials(true);
    m_model = loader.Load("models/mill/Mill.obj");

    Texture2DLoader texture2DLoader(TextureObject::FormatRGBA, TextureObject::InternalFormatRGBA8);
    texture2DLoader.SetFlipVertical(true);

    auto shadowTexture = std::make_shared<Texture2DObject>(texture2DLoader.Load("models/mill/Ground_shadow.jpg"));
    auto groundTexture = std::make_shared<Texture2DObject>(texture2DLoader.Load("models/mill/Ground_color.jpg"));
    auto millTexture = std::make_shared<Texture2DObject>(texture2DLoader.Load("models/mill/MillCat_color.jpg"));

    m_model.GetMaterial(0).SetUniformValue("Albedo", shadowTexture);
    m_model.GetMaterial(1).SetUniformValue("Albedo", groundTexture);
    m_model.GetMaterial(2).SetUniformValue("Albedo", millTexture);
}

void CloudApplication::InitCube() {
    m_cloudModel.SetMesh(m_cloudMesh);
    m_cloudModel.AddMaterial(m_cloudMaterial);

    glm::mat4 translate = glm::translate(glm::vec3(0.0f, 150.0f, 0.0f));
    m_worldMatrix = glm::scale(translate, glm::vec3(200.0f, 200.0f, 200.0f));
    m_cloudModel.GetMaterial(0).SetUniformValue("WorldMatrix", m_worldMatrix);
    m_sunModel.SetMesh(m_cloudMesh);
    m_sunModel.AddMaterial(m_sunMaterial);
}


void CloudApplication::Cleanup() {
    Application::Cleanup();
    m_imGui.Cleanup();
}