#pragma once

#include <ituGL/application/Application.h>

#include <ituGL/scene/Scene.h>
#include <ituGL/renderer/Renderer.h>
#include <ituGL/camera/CameraController.h>
#include <ituGL/utils/DearImGui.h>

class TextureCubemapObject;
class Material;
class Texture2DObject;

class RefractionApp : public Application
{
public:
    RefractionApp();

protected:
    void Initialize() override;
    void Update() override;
    void Render() override;
    void Cleanup() override;

private:
    void InitializeCamera();
    void InitializeModels();
    std::shared_ptr<Material> InitializeMaterial(std::vector<const char*> vertexShaderPaths, std::vector<const char*> fragmentShaderPaths);
    void InitializeRenderer();
    void InitializeMaterials();
    void InitializeShaders();
    void InitializeFramebuffers();
    

    std::shared_ptr<Mesh> CreatePlaneMesh(int width, int depth, float spacing);

    std::shared_ptr<Mesh> CreatePlaneFromImage( const char* path, float heightScale, float spacing, bool use_height);


    std::shared_ptr<Mesh> CreateCubeMesh();

    void RenderGUI();
private:
    // Helper object for debug GUI
    DearImGui m_imGui;

    // Camera controller
    CameraController m_cameraController;

    // Global scene
    Scene m_scene;

    // Renderer
    Renderer m_renderer;

    // Skybox texture
    std::shared_ptr<TextureCubemapObject> m_skyboxTexture;

    // Default material
    std::shared_ptr<Material> m_groundMaterial;
    std::shared_ptr<Material> m_waterMaterial;
    std::shared_ptr<Model> m_groundTexture;

    std::shared_ptr<TextureObject> m_waterNormal;
    std::shared_ptr<Texture2DObject> m_sceneTexture;
    glm::vec2 planeSize;
};