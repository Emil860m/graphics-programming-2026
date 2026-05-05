#include "RefractionApp.h"
//#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <ituGL/asset/TextureCubemapLoader.h>
#include <ituGL/texture/Texture2DObject.h>
#include <ituGL/asset/Texture2DLoader.h>
#include <ituGL/asset/ShaderLoader.h>
#include <ituGL/asset/ModelLoader.h>

#include <ituGL/camera/Camera.h>
#include <ituGL/scene/SceneCamera.h>

#include <ituGL/lighting/DirectionalLight.h>
#include <ituGL/lighting/PointLight.h>
#include <ituGL/scene/SceneLight.h>

#include <ituGL/shader/ShaderUniformCollection.h>
#include <ituGL/shader/Material.h>
#include <ituGL/geometry/Model.h>
#include <ituGL/scene/SceneModel.h>

#include <ituGL/renderer/SkyboxRenderPass.h>
#include <ituGL/renderer/ForwardRenderPass.h>
#include <ituGL/scene/RendererSceneVisitor.h>
#include <ituGL/renderer/DeferredRenderPass.h>
#include <ituGL/renderer/PostFXRenderPass.h>
#include <ituGL/renderer/GBufferRenderPass.h>

#include <ituGL/scene/ImGuiSceneVisitor.h>
#include <imgui.h>

#include <iostream>
using namespace std;
RefractionApp::RefractionApp()
    : Application(1024, 1024, "Scene Viewer demo")
    , m_renderer(GetDevice())
{
}

void RefractionApp::Initialize()
{
    Application::Initialize();
    
    // Initialize DearImGUI
    m_imGui.Initialize(GetMainWindow());
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    InitializeCamera();
    InitializeMaterials();
    InitializeModels();
    InitializeRenderer();
    glfwSetTime(0.0);
}

void RefractionApp::Update()
{
    Application::Update();
    cout << GetDeltaTime() << "\n";
    delta_time += GetDeltaTime();

    // Update camera controller
    m_cameraController.Update(GetMainWindow(), GetDeltaTime());

    // Add the scene nodes to the renderer
    RendererSceneVisitor rendererSceneVisitor(m_renderer);
    m_scene.AcceptVisitor(rendererSceneVisitor);
}


void RefractionApp::Render()
{
    Application::Render();

    GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);

    // Render the scene
    m_renderer.Render();

    // Render the debug user interface
    RenderGUI();

}

void RefractionApp::Cleanup()
{
    // Cleanup DearImGUI
    m_imGui.Cleanup();

    Application::Cleanup();
}

void RefractionApp::InitializeCamera()
{
    // Create the main camera
    std::shared_ptr<Camera> camera = std::make_shared<Camera>();
    camera->SetViewMatrix(glm::vec3(-1, 1, 1), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));
    camera->SetPerspectiveProjectionMatrix(1.0f, 1.0f, 0.1f, 100.0f);

    // Create a scene node for the camera
    std::shared_ptr<SceneCamera> sceneCamera = std::make_shared<SceneCamera>("camera", camera);

    // Add the camera node to the scene
    m_scene.AddSceneNode(sceneCamera);

    // Set the camera scene node to be controlled by the camera controller
    m_cameraController.SetCamera(sceneCamera);
}
void RefractionApp::InitializeMaterials() {
    {
        std::vector<const char*> waterVertexShaders;
        waterVertexShaders.push_back("shaders/basic.vert");
        std::vector<const char*> waterFragmentShaders;
        waterFragmentShaders.push_back("shaders/utils.glsl");
        waterFragmentShaders.push_back("shaders/basic.frag");
        
        m_waterMaterial = InitializeMaterial(waterVertexShaders, waterFragmentShaders, true);
        m_waterMaterial->SetBlendEquation(Material::BlendEquation::Add);
        m_waterMaterial->SetBlendParams(
            Material::BlendParam::SourceAlpha,
            Material::BlendParam::OneMinusSourceAlpha
        );
        m_waterMaterial->SetDepthTestFunction(Material::TestFunction::LessEqual);
    }
    {
        std::vector<const char*> groundVertexShaders;
        std::vector<const char*> groundFragmentShaders;
        groundVertexShaders.push_back("shaders/simple.vert");
        groundFragmentShaders.push_back("shaders/simple.frag");

        m_groundMaterial = InitializeMaterial(groundVertexShaders, groundFragmentShaders, false);
        Texture2DLoader textureLoader(TextureObject::FormatRGBA, TextureObject::InternalFormatRGBA8);
        std::shared_ptr<Texture2DObject> texture = textureLoader.LoadShared("textures/4429.jpg");
        m_waterMaterial->SetUniformValue("groundPlane", texture);
        m_groundMaterial->SetUniformValue("tex", texture);
        m_groundMaterial->SetUniformValue("Color", glm::vec3(0.8f, 0.7f, 0.5f));
        m_groundMaterial->SetBlendEquation(Material::BlendEquation::None);
        m_groundMaterial->SetDepthWrite(true);
    }
    //m_waterMaterial->SetDepthWrite(false);
}

std::shared_ptr<Material> RefractionApp::InitializeMaterial(std::vector<const char*> vertexShaderPaths, std::vector<const char*> fragmentShaderPaths, bool time)
{
  // Load and build shader
    vertexShaderPaths.insert(vertexShaderPaths.begin(), "shaders/version330.glsl");
    Shader vertexShader = ShaderLoader(Shader::VertexShader).Load(vertexShaderPaths);

    fragmentShaderPaths.insert(fragmentShaderPaths.begin(), "shaders/version330.glsl");
    Shader fragmentShader = ShaderLoader(Shader::FragmentShader).Load(fragmentShaderPaths);

    std::shared_ptr<ShaderProgram> shaderProgramPtr = std::make_shared<ShaderProgram>();
    shaderProgramPtr->Build(vertexShader, fragmentShader);

    // Get transform related uniform locations
    ShaderProgram::Location cameraPositionLocation = shaderProgramPtr->GetUniformLocation("CameraPosition");
    ShaderProgram::Location worldMatrixLocation = shaderProgramPtr->GetUniformLocation("WorldMatrix");
    ShaderProgram::Location viewProjMatrixLocation = shaderProgramPtr->GetUniformLocation("ViewProjMatrix");
    ShaderProgram::Location timeLocation = shaderProgramPtr->GetUniformLocation("Time");
    m_renderer.RegisterShaderProgram(shaderProgramPtr,
        [=](const ShaderProgram& shaderProgram, const glm::mat4& worldMatrix, const Camera& camera, bool cameraChanged)
        {
            if (cameraChanged)
            {
                shaderProgram.SetUniform(cameraPositionLocation, camera.ExtractTranslation());
                shaderProgram.SetUniform(viewProjMatrixLocation, camera.GetViewProjectionMatrix());
            }
            if (time)
                shaderProgram.SetUniform(timeLocation, delta_time);
            shaderProgram.SetUniform(worldMatrixLocation, worldMatrix);
        },
        m_renderer.GetDefaultUpdateLightsFunction(*shaderProgramPtr)
    );

    // Create reference material
    assert(shaderProgramPtr);
    return std::make_shared<Material>(shaderProgramPtr);
    
    
}



void RefractionApp::InitializeModels()
{

    // Skybox
    m_skyboxTexture = TextureCubemapLoader::LoadTextureShared("models/defaultCubemap.png", TextureObject::FormatRGB, TextureObject::InternalFormatSRGB8);
    m_skyboxTexture->Bind();
    float maxLod;
    m_skyboxTexture->GetParameter(TextureObject::ParameterFloat::MaxLod, maxLod);
    TextureCubemapObject::Unbind();

    // Ground
    std::shared_ptr<Mesh> groundMesh = CreatePlaneFromImage("noise_maps/iceland_heightmap.png", 1.0f, 0.01f, true);
    std::shared_ptr<Model> groundModel = std::make_shared<Model>(groundMesh);
    groundModel->AddMaterial(m_groundMaterial);
    std::shared_ptr<SceneModel> groundNode = std::make_shared<SceneModel>("ground", groundModel);
    //m_groundTexture = groundModel;

    std::shared_ptr<Mesh> waterMesh = CreatePlaneFromImage("noise_maps/iceland_heightmap.png", 0.5f, 0.01f, false);
    std::shared_ptr<Model> waterModel = std::make_shared<Model>(waterMesh);
    waterModel->AddMaterial(m_waterMaterial);
    std::shared_ptr<SceneModel> waterNode = std::make_shared<SceneModel>("water", waterModel);
    m_scene.AddSceneNode(waterNode);
    m_scene.AddSceneNode(groundNode);
    

}

void RefractionApp::InitializeFramebuffers() {
    int width, height;
    GetMainWindow().GetDimensions(width, height);
    // Scene Texture
    m_sceneTexture = std::make_shared<Texture2DObject>();
    m_sceneTexture->Bind();
    m_sceneTexture->SetImage(0, width, height, TextureObject::FormatRGBA, TextureObject::InternalFormat::InternalFormatRGBA16F);
    m_sceneTexture->SetParameter(TextureObject::ParameterEnum::MinFilter, GL_LINEAR);
    m_sceneTexture->SetParameter(TextureObject::ParameterEnum::MagFilter, GL_LINEAR);
    Texture2DObject::Unbind();
    
        // Scene framebuffer
    //assert(false);
    m_sceneFramebuffer = std::make_shared<FramebufferObject>();
    m_sceneFramebuffer->Bind();
    m_sceneFramebuffer->SetTexture(FramebufferObject::Target::Draw, FramebufferObject::Attachment::Depth, *m_depthTexture);
    m_sceneFramebuffer->SetTexture(FramebufferObject::Target::Draw, FramebufferObject::Attachment::Color0, *m_sceneTexture);
    m_sceneFramebuffer->SetDrawBuffers(std::array<FramebufferObject::Attachment, 1>({ FramebufferObject::Attachment::Color0 }));
    FramebufferObject::Unbind();
    
    Texture2DObject::Unbind();
    FramebufferObject::Unbind();

}

void RefractionApp::InitializeRenderer()
{
    unsigned int m_opaqueCollection = m_renderer.AddDrawcallCollection(
        [](const Renderer::DrawcallInfo& dc)
        {
            return !dc.GetMaterial().HasBlend();
        }
    );
    unsigned int m_transparentCollection = m_renderer.AddDrawcallCollection(
        [](const Renderer::DrawcallInfo& dc)
        {
            return dc.GetMaterial().HasBlend();
        }
    );   
    /*
    int width, height;
    GetMainWindow().GetDimensions(width, height);
    std::unique_ptr<GBufferRenderPass> opaqueRenderPass(std::make_unique<GBufferRenderPass>(width, height, m_opaqueCollection));
    m_depthTexture = opaqueRenderPass->GetDepthTexture();

    m_waterMaterial->SetUniformValue("DepthTexture", opaqueRenderPass->GetDepthTexture());
    m_waterMaterial->SetUniformValue("groundPlane", opaqueRenderPass->GetAlbedoTexture());
    //std::shared_ptr<Texture2DObject> albedo = opaqueRenderPass->GetAlbedoTexture();
    m_renderer.AddRenderPass(std::move(opaqueRenderPass));
    m_renderer.AddRenderPass(std::make_unique<DeferredRenderPass>(m_waterMaterial, m_sceneFramebuffer));
    InitializeFramebuffers();*/
    //m_renderer.SetCurrentFramebuffer(m_renderer.GetCurrentFramebuffer());
    m_renderer.AddRenderPass(std::make_unique<SkyboxRenderPass>(m_skyboxTexture));
    m_renderer.AddRenderPass(std::make_unique<ForwardRenderPass>(m_opaqueCollection));
    m_renderer.AddRenderPass(std::make_unique<ForwardRenderPass>(m_transparentCollection));
    //std::shared_ptr<Material> copyMaterial = CreatePostFXMaterial("shaders/renderer/copy.frag", m_sceneTexture);
    //m_renderer.AddRenderPass(std::make_unique<PostFXRenderPass>(copyMaterial, m_renderer.GetDefaultFramebuffer()));
    glDisable(GL_CULL_FACE);
}

std::shared_ptr<Material> RefractionApp::CreatePostFXMaterial(const char* fragmentShaderPath, std::shared_ptr<Texture2DObject> sourceTexture)
{
    // We could keep this vertex shader and reuse it, but it looks simpler this way
    std::vector<const char*> vertexShaderPaths;
    vertexShaderPaths.push_back("shaders/version330.glsl");
    vertexShaderPaths.push_back("shaders/renderer/fullscreen.vert");
    Shader vertexShader = ShaderLoader(Shader::VertexShader).Load(vertexShaderPaths);

    std::vector<const char*> fragmentShaderPaths;
    fragmentShaderPaths.push_back("shaders/version330.glsl");
    fragmentShaderPaths.push_back("shaders/utils.glsl");
    fragmentShaderPaths.push_back(fragmentShaderPath);
    Shader fragmentShader = ShaderLoader(Shader::FragmentShader).Load(fragmentShaderPaths);

    std::shared_ptr<ShaderProgram> shaderProgramPtr = std::make_shared<ShaderProgram>();
    shaderProgramPtr->Build(vertexShader, fragmentShader);

    // Create material
    std::shared_ptr<Material> material = std::make_shared<Material>(shaderProgramPtr);
    material->SetUniformValue("SourceTexture", sourceTexture);
    
    return material;
}

void RefractionApp::RenderGUI()
{
    m_imGui.BeginFrame();

    // Draw GUI for scene nodes, using the visitor pattern
    ImGuiSceneVisitor imGuiVisitor(m_imGui, "Scene");
    m_scene.AcceptVisitor(imGuiVisitor);
    
    // Draw GUI for camera controller
    m_cameraController.DrawGUI(m_imGui);

    m_imGui.EndFrame();
}

std::shared_ptr<Mesh> RefractionApp::CreatePlaneMesh(int width, int depth, float spacing)
{
    std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();

    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
    };

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // --- Generate vertices ---
    for (int z = 0; z <= depth; ++z)
    {
        for (int x = 0; x <= width; ++x)
        {
            float xpos = x * spacing - (width * spacing * 0.5f);
            float zpos = z * spacing - (depth * spacing * 0.5f);
            //float y = sin(x);
            float y = 0.0f;
            vertices.push_back({
                glm::vec3(xpos, y, zpos),
                glm::vec3(0.0f, 1.0f, 0.0f)
            });
        }
    }

    // --- Generate indices ---
    for (int z = 0; z < depth; ++z)
    {
        for (int x = 0; x < width; ++x)
        {
            int i0 = z * (width + 1) + x;
            int i1 = i0 + 1;
            int i2 = i0 + (width + 1);
            int i3 = i2 + 1;

            indices.push_back(i0);
            indices.push_back(i2);
            indices.push_back(i1);

            indices.push_back(i1);
            indices.push_back(i2);
            indices.push_back(i3);
        }
    }

    // --- Layout ---
    std::vector<VertexAttribute::Layout> layout = {
        VertexAttribute::Layout(
            VertexAttribute(Data::Type::Float, 3, VertexAttribute::Semantic::Position),
            offsetof(Vertex, position),
            sizeof(Vertex)
        ),
        VertexAttribute::Layout(
            VertexAttribute(Data::Type::Float, 3, VertexAttribute::Semantic::Normal),
            offsetof(Vertex, normal),
            sizeof(Vertex)
        )
    };

    mesh->AddSubmesh<Vertex, unsigned int>(
        Drawcall::Primitive::Triangles,
        std::span<const Vertex>(vertices.data(), vertices.size()),
        std::span<const unsigned int>(indices.data(), indices.size()),
        layout.begin(),
        layout.end()
    );

    return mesh;
}
std::shared_ptr<Mesh> RefractionApp::CreateCubeMesh()  {
    std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
    };
    std::vector<Vertex> vertices = {/*
    // Front (+Z)
    {{-0.5f,-0.5f, 0.5f}, {0,0,1}},
    {{ 0.5f,-0.5f, 0.5f}, {0,0,1}},
    {{ 0.5f, 0.5f, 0.5f}, {0,0,1}},
    {{-0.5f, 0.5f, 0.5f}, {0,0,1}},

    // Back (-Z)
    {{-0.5f,-0.5f,-0.5f}, {0,0,-1}},
    {{ 0.5f,-0.5f,-0.5f}, {0,0,-1}},
    {{ 0.5f, 0.5f,-0.5f}, {0,0,-1}},
    {{-0.5f, 0.5f,-0.5f}, {0,0,-1}},

    // Left (-X)
    {{-0.5f,-0.5f,-0.5f}, {-1,0,0}},
    {{-0.5f,-0.5f, 0.5f}, {-1,0,0}},
    {{-0.5f, 0.5f, 0.5f}, {-1,0,0}},
    {{-0.5f, 0.5f,-0.5f}, {-1,0,0}},

    // Right (+X)
    {{ 0.5f,-0.5f,-0.5f}, {1,0,0}},
    {{ 0.5f,-0.5f, 0.5f}, {1,0,0}},
    {{ 0.5f, 0.5f, 0.5f}, {1,0,0}},
    {{ 0.5f, 0.5f,-0.5f}, {1,0,0}},

    // Top (+Y)
    {{-0.5f, 0.5f,-0.5f}, {0,1,0}},
    {{ 0.5f, 0.5f,-0.5f}, {0,1,0}},
    {{ 0.5f, 0.5f, 0.5f}, {0,1,0}},
    {{-0.5f, 0.5f, 0.5f}, {0,1,0}},*/

    // Bottom (-Y)
    {{-0.5f,-40.0f,-0.5f}, {0,-1,0}},
    {{ 0.5f,-40.0f,-0.5f}, {0,-1,0}},
    {{ 0.5f,100.0f, 0.5f}, {0,-1,0}},
    {{-0.5f,100.0f, 0.5f}, {0,-1,0}},
    
};
    
    /*
    {
        // positions          // normals
        {{-0.5f, -0.5f, -0.5f},  {0.0f,  0.0f, -1.0f}},
        {{ 0.5f, -0.5f, -0.5f},  {0.0f,  0.0f, -1.0f}},
        {{ 0.5f,  0.5f, -0.5f},  {0.0f,  0.0f, -1.0f}},
        {{ 0.5f,  0.5f, -0.5f},  {0.0f,  0.0f, -1.0f}},
        {{-0.5f,  0.5f, -0.5f},  {0.0f,  0.0f, -1.0f}},
        {{-0.5f, -0.5f, -0.5f},  {0.0f,  0.0f, -1.0f}},

        {{-0.5f, -0.5f,  0.5f},  {0.0f,  0.0f, 1.0f}},
        {{ 0.5f, -0.5f,  0.5f},  {0.0f,  0.0f, 1.0f}},
        {{ 0.5f,  0.5f,  0.5f},  {0.0f,  0.0f, 1.0f}},
        {{ 0.5f,  0.5f,  0.5f},  {0.0f,  0.0f, 1.0f}},
        {{-0.5f,  0.5f,  0.5f},  {0.0f,  0.0f, 1.0f}},
        {{-0.5f, -0.5f,  0.5f},  {0.0f,  0.0f, 1.0f}},

        {{-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}},
        {{-0.5f,  0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}},
        {{-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}},
        {{-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}},
        {{-0.5f, -0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}},
        {{-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}},

        {{ 0.5f,  0.5f,  0.5f},  {1.0f,  0.0f,  0.0f}},
        {{ 0.5f,  0.5f, -0.5f},  {1.0f,  0.0f,  0.0f}},
        {{ 0.5f, -0.5f, -0.5f},  {1.0f,  0.0f,  0.0f}},
        {{ 0.5f, -0.5f, -0.5f},  {1.0f,  0.0f,  0.0f}},
        {{ 0.5f, -0.5f,  0.5f},  {1.0f,  0.0f,  0.0f}},
        {{ 0.5f,  0.5f,  0.5f},  {1.0f,  0.0f,  0.0f}},

        {{-0.5f, -0.5f, -0.5f},  {0.0f, -1.0f,  0.0f}},
        {{ 0.5f, -0.5f, -0.5f},  {0.0f, -1.0f,  0.0f}},
        {{ 0.5f, -0.5f,  0.5f},  {0.0f, -1.0f,  0.0f}},
        {{ 0.5f, -0.5f,  0.5f},  {0.0f, -1.0f,  0.0f}},
        {{-0.5f, -0.5f,  0.5f},  {0.0f, -1.0f,  0.0f}},
        {{-0.5f, -0.5f, -0.5f},  {0.0f, -1.0f,  0.0f}},

        {{-0.5f,  0.5f, -0.5f},  {0.0f,  1.0f,  0.0f}},
        {{ 0.5f,  0.5f, -0.5f},  {0.0f,  1.0f,  0.0f}},
        {{ 0.5f,  0.5f,  0.5f},  {0.0f,  1.0f,  0.0f}},
        {{ 0.5f,  0.5f,  0.5f},  {0.0f,  1.0f,  0.0f}},
        {{-0.5f,  0.5f,  0.5f},  {0.0f,  1.0f,  0.0f}},
        {{-0.5f,  0.5f, -0.5f},  {0.0f,  1.0f,  0.0f}},
    };
    */

    std::vector<unsigned int> indices = {
        0,1,2, 2,3,0,
        4,5,6, 6,7,4,
        8,9,10, 10,11,8,
        12,13,14, 14,15,12,
        16,17,18, 18,19,16,
        20,21,22, 22,23,20
    };

    // --- Correct layout construction ---
    VertexAttribute positionAttr(
        Data::Type::Float,     // type
        3,                     // vec3
        VertexAttribute::Semantic::Position
    );

    std::vector<VertexAttribute::Layout> layout = {
        VertexAttribute::Layout(
            VertexAttribute(Data::Type::Float, 3, VertexAttribute::Semantic::Position),
            offsetof(Vertex, position),
            sizeof(Vertex)
        ),
        VertexAttribute::Layout(
            VertexAttribute(Data::Type::Float, 3, VertexAttribute::Semantic::Normal),
            offsetof(Vertex, normal),
            sizeof(Vertex)
        )
    };
    mesh->AddSubmesh(
        Drawcall::Primitive::Triangles,
        std::span<const Vertex>(vertices),
        std::span<const unsigned int>(indices),
        layout.begin(),
        layout.end()
    );
    return mesh;
}

float GetBrightness(unsigned char r, unsigned char g, unsigned char b)
{
    return (0.2126f * r + 0.7152f * g + 0.0722f * b) / 255.0f;
}

std::shared_ptr<Mesh> RefractionApp::CreatePlaneFromImage(
    const char* path,
    float heightScale,
    float spacing,
    bool use_height)
{
    int width, height, channels;
    //stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &width, &height, &channels, 3);
    if (!data)
    {
        throw std::runtime_error("Failed to load image");
    }
    planeSize = glm::vec2(width * spacing, height * spacing);
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;
    };

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<float> heights(width * height);

    for (int z = 0; z < height; ++z)
    {
        for (int x = 0; x < width; ++x)
        {
            int idx = (z * width + x) * 3;

            float brightness = GetBrightness(
                data[idx],
                data[idx + 1],
                data[idx + 2]
            );

            heights[z * width + x] = brightness * heightScale;
        }
    }
    

        // Helper
    auto getHeight = [&](int x, int z)
    {
        x = glm::clamp(x, 0, width - 1);
        z = glm::clamp(z, 0, height - 1);
        return heights[z * width + x];
    };


    // --- vertices ---
    for (int z = 0; z < height; ++z)
    {
        for (int x = 0; x < width; ++x)
        {
            float hL = getHeight(x - 1, z);
            float hR = getHeight(x + 1, z);
            float hD = getHeight(x, z - 1);
            float hU = getHeight(x, z + 1);

            // slope
            float dx = (hR - hL) / (2.0f * spacing);
            float dz = (hU - hD) / (2.0f * spacing);
            float u = (float)x / (width - 1);
            float v = (float)z / (height - 1);
            // build normal
            glm::vec3 normal = glm::normalize(glm::vec3(-dx, 1.0f, -dz));
            //glm::vec3 normal = glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f));

            float xpos = x * spacing - (width * spacing * 0.5f);
            float zpos = z * spacing - (height * spacing * 0.5f);
            float ypos = 0.5f;
            if (use_height) {
                ypos = heights[z * width + x];
            }

            vertices.push_back({
                glm::vec3(xpos, ypos, zpos),
                normal,
                glm::vec2(u, v)
            });
        }
    }
    

    // --- indices ---
    for (int z = 0; z < height - 1; ++z)
    {
        for (int x = 0; x < width - 1; ++x)
        {
            unsigned int i0 = z * width + x;
            unsigned int i1 = i0 + 1;
            unsigned int i2 = i0 + width;
            unsigned int i3 = i2 + 1;

            indices.insert(indices.end(), {
                i0, i2, i1,
                i1, i2, i3
            });
        }
    }

    stbi_image_free(data);

    // --- layout ---
    std::vector<VertexAttribute::Layout> layout = {
        VertexAttribute::Layout(
            VertexAttribute(Data::Type::Float, 3, VertexAttribute::Semantic::Position),
            offsetof(Vertex, position),
            sizeof(Vertex)
        ),
        VertexAttribute::Layout(
            VertexAttribute(Data::Type::Float, 3, VertexAttribute::Semantic::Normal),
            offsetof(Vertex, normal),
            sizeof(Vertex)
        ),
        VertexAttribute::Layout(
            VertexAttribute(Data::Type::Float, 2, VertexAttribute::Semantic::TexCoord0),
            offsetof(Vertex, uv),
            sizeof(Vertex)
        )
    };

    auto mesh = std::make_shared<Mesh>();

    mesh->AddSubmesh<Vertex, unsigned int>(
        Drawcall::Primitive::Triangles,
        std::span(vertices.data(), vertices.size()),
        std::span(indices.data(), indices.size()),
        layout.begin(),
        layout.end()
    );

    return mesh;
}