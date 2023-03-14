#pragma once

#include <ituGL/application/Application.h>

#include <ituGL/camera/Camera.h>
#include <ituGL/geometry/Model.h>
#include <ituGL/utils/DearImGui.h>
#include <ituGL/geometry/Mesh.h>

class Texture2DObject;

class ViewerApplication : public Application
{
public:
    ViewerApplication();

protected:
    void Initialize() override;
    void Update() override;
    void Render() override;
    void Cleanup() override;

private:
    void InitializeModel();
    void InitializeQuad();
    void InitializeCamera();
    void InitializeLights();

    void UpdateCamera();
    void UpdateMaterials();

    void RenderGUI();
    void ShadowPass();
    void InitializeShadows();
    void InitializeShadowTexture();

    void CreateQuadMesh(std::shared_ptr<Mesh> mesh);

private:
    // Helper object for debug GUI
    DearImGui m_imGui;

    // Mouse position for camera controller
    glm::vec2 m_mousePosition;

    // Camera controller parameters
    Camera m_camera;
    glm::vec3 m_cameraPosition;
    float m_cameraTranslationSpeed;
    float m_cameraRotationSpeed;
    bool m_cameraEnabled;
    bool m_cameraEnablePressed;

    // Loaded model
    Model m_model;
    Model m_shadowModel;
    std::shared_ptr<Mesh> m_quadMesh;
    Model m_quadModel;
    unsigned int depthMap;
    unsigned int depthMapFBO;
    glm::mat4 m_lightSpaceMatrix;
    float m_drawShadow = 0.0f;

    // Materials
    std::shared_ptr<Material> m_phongShader;
    std::shared_ptr<Material> m_shadowMaterial;

    // Shadows
    int m_shadowWidth = 1024 * 2;
    int m_shadowHeight = 1024 * 2;
    int m_screenWidth = 800 * 2;
    int m_screenHeight = 800 * 2;

    float m_nearPlane = 1.0f;
    float m_farPlane = 400.0f;

    // (todo) 05.X: Add light variables
    glm::vec3 m_ambientColor = glm::vec3(1.0);
    glm::vec3 m_lightColor = glm::vec3(1.0f, 0.95, 0.4);
    glm::vec3 m_lightPosition = glm::vec3 (-65.0f, 26.0f, 0.0f);
    float m_lightIntensity = 1.0f;

    // Material
    float m_ambientReflectionShadow = 0.25f;
    float m_diffuseReflectionShadow = 1.0f;
    float m_specularReflectionShadow = 1.0f;
    float m_specularExponentShadow = 1.0f;

    float m_ambientReflectionGround = 0.25f;
    float m_diffuseReflectionGround = 1.0f;
    float m_specularReflectionGround = 1.0f;
    float m_specularExponentGround = 50.0f;

    float m_ambientReflectionMill = 0.25f;
    float m_diffuseReflectionMill = 1.0f;
    float m_specularReflectionMill = 1.0f;
    float m_specularExponentMill = 100.0f;

    // Textures
    std::shared_ptr<Texture2DObject> m_shadowTexture;
    std::shared_ptr<Texture2DObject> m_groundTexture;
    std::shared_ptr<Texture2DObject> m_millTexture;
};
