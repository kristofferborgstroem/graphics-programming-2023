#pragma once

#include <ituGL/application/Application.h>

#include <ituGL/asset/ShaderLoader.h>
#include <ituGL/geometry/Mesh.h>
#include <ituGL/camera/Camera.h>
#include <ituGL/shader/Material.h>
#include <glm/mat4x4.hpp>
#include <vector>

class Texture2DObject;

class TexturedTerrainApplication : public Application
{
public:
    TexturedTerrainApplication();

protected:
    void Initialize() override;
    void Update() override;
    void Render() override;

private:
    void InitializeTextures();
    void InitializeMaterials();
    void InitializeMeshes();

    void DrawObject(const Mesh& mesh, Material& material, const glm::mat4& worldMatrix);

    std::shared_ptr<Texture2DObject> CreateDefaultTexture();
    std::shared_ptr<Texture2DObject> CreateHeightMap(unsigned int width, unsigned int height, glm::ivec2 coords);
    std::shared_ptr<Texture2DObject> LoadTexture(const char* path);

    void CreateTerrainMesh(Mesh& mesh, unsigned int gridX, unsigned int gridY);

private:
    unsigned int m_gridX, m_gridY;

    glm::vec2 lastMouse;
    float mouseSensitivity = 100.0f;
    float yaw = 0.0f;
    float pitch = 0.0f;

    Camera m_camera;

    ShaderLoader m_vertexShaderLoader;
    ShaderLoader m_fragmentShaderLoader;

    Mesh m_terrainPatch;
    std::shared_ptr<Material> m_defaultMaterial;

    // (todo) 04.X: Add additional materials
    std::shared_ptr<Material> m_terrainMaterial1;
    std::shared_ptr<Material> m_terrainMaterial2;
    std::shared_ptr<Material> m_terrainMaterial3;
    std::shared_ptr<Material> m_terrainMaterial4;

    std::shared_ptr<Material> m_waterMaterial;


    std::shared_ptr<Texture2DObject> m_defaultTexture;

    // (todo) 04.X: Add additional textures
    std::shared_ptr<Texture2DObject> m_grassTexture;
    std::shared_ptr<Texture2DObject> m_rockTexture;
    std::shared_ptr<Texture2DObject> m_snowTexture;
    std::shared_ptr<Texture2DObject> m_dirtTexture;
    std::shared_ptr<Texture2DObject> m_waterTexture;
};
