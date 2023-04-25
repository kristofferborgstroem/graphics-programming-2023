//
// Created by Kristoffer Borgstrøm on 21/03/2023.
//
#pragma once

#include <ituGL/application/Application.h>
#include <ituGL/shader/Material.h>
#include <ituGL/asset/ModelLoader.h>
#include <ituGL/texture/Texture3DObject.h>
#include <ituGL/utils/DearImGui.h>

#include "Controller.hpp"
#include "Cloud.hpp"


class CloudApplication : public Application
{
public:
    CloudApplication();

protected:
    void Initialize() override;
    void Update() override;
    void Render() override;
    void Cleanup() override;

    void RenderGUI();

private:
    void InitModel();
    void InitCube();
    void InitializeShaders();
    void InitializeCloudShader();
    void InitializeUnlitShader();
    void InitializeSunShader();

private:

    // GUI
    DearImGui m_imGui;

    // Cloud
    Cloud m_cloud;
    std::shared_ptr<Texture3DObject> m_cloudNoise;
    std::shared_ptr<Texture3DObject> m_cloudNoiseLowRes;
    float m_cloudMarchAbsorption = 1.3f;
    float m_lightMarchAbsorption = 1.2f;
    float m_scale = 1.0;
    float m_weatherScale = 1.0;
    int m_debug = 1;
    float m_animationSpeed = 0.0f;

    int m_cloudSteps = 20;
    int m_lightSteps = 8;
    float m_forwardScattering = 0.2f;
    float m_backwardScattering = 0.2f;
    float m_thickness = 1.0f;
    glm::vec3 m_cloudSampling = glm::vec3(0.625, 0.25, 0.125);

    // Model & material
    std::shared_ptr<Material> m_cloudMaterial;
    std::shared_ptr<Material> m_unlitMaterial;
    std::shared_ptr<Material> m_sunMaterial;


    // Lights
    glm::vec3 m_lightPosition = glm::vec3(30.0f, 30.0f, 30.0f);

    glm::mat4 m_worldMatrix;
    Model m_model;
    std::shared_ptr<Mesh> m_cloudMesh;
    Model m_cloudModel;
    Model m_sunModel;

    int m_srcWidth = 1024;
    int m_srcHeight = 1024;

    // Camera
    Controller controller;
};

