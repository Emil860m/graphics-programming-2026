#include "RefractionApp.h"

#include <ituGL/asset/TextureCubemapLoader.h>
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

#include <ituGL/scene/ImGuiSceneVisitor.h>
#include <imgui.h>

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
    glDepthMask(GL_FALSE);
    InitializeCamera();
    InitializeMaterial();
    InitializeModels();
    InitializeRenderer();
}

void RefractionApp::Update()
{
    Application::Update();

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
    camera->SetViewMatrix(glm::vec3(-1, 1, 1), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    camera->SetPerspectiveProjectionMatrix(1.0f, 1.0f, 0.1f, 100.0f);

    // Create a scene node for the camera
    std::shared_ptr<SceneCamera> sceneCamera = std::make_shared<SceneCamera>("camera", camera);

    // Add the camera node to the scene
    m_scene.AddSceneNode(sceneCamera);

    // Set the camera scene node to be controlled by the camera controller
    m_cameraController.SetCamera(sceneCamera);
}

void RefractionApp::InitializeMaterial()
{
  // Load and build shader
    std::vector<const char*> vertexShaderPaths;
    vertexShaderPaths.push_back("shaders/version330.glsl");
    vertexShaderPaths.push_back("shaders/simple.vert");

    Shader vertexShader = ShaderLoader(Shader::VertexShader).Load(vertexShaderPaths);

    std::vector<const char*> fragmentShaderPaths;
    fragmentShaderPaths.push_back("shaders/version330.glsl");
    fragmentShaderPaths.push_back("shaders/simple.frag");
    Shader fragmentShader = ShaderLoader(Shader::FragmentShader).Load(fragmentShaderPaths);

    std::shared_ptr<ShaderProgram> shaderProgramPtr = std::make_shared<ShaderProgram>();
    shaderProgramPtr->Build(vertexShader, fragmentShader);

    // Get transform related uniform locations
    ShaderProgram::Location cameraPositionLocation = shaderProgramPtr->GetUniformLocation("CameraPosition");
    ShaderProgram::Location worldMatrixLocation = shaderProgramPtr->GetUniformLocation("WorldMatrix");
    ShaderProgram::Location viewProjMatrixLocation = shaderProgramPtr->GetUniformLocation("ViewProjMatrix");
    //auto skyboxLoc = shaderProgramPtr->GetUniformLocation("Skybox");
    //auto timeLoc   = shaderProgramPtr->GetUniformLocation("Time");
    //shaderProgramPtr->SetTexture(skyboxLoc, 0, *m_skyboxTexture);
    // Register shader with renderer
    m_renderer.RegisterShaderProgram(shaderProgramPtr,
        [=](const ShaderProgram& shaderProgram, const glm::mat4& worldMatrix, const Camera& camera, bool cameraChanged)
        {
            if (cameraChanged)
            {
                shaderProgram.SetUniform(cameraPositionLocation, camera.ExtractTranslation());
                shaderProgram.SetUniform(viewProjMatrixLocation, camera.GetViewProjectionMatrix());
                float time = (float)glfwGetTime(); // or your engine time

                //shaderProgram.SetUniform(timeLoc, time);
            }
            shaderProgram.SetUniform(worldMatrixLocation, worldMatrix);
        },
        m_renderer.GetDefaultUpdateLightsFunction(*shaderProgramPtr)
    );
    
    // Create reference material
    assert(shaderProgramPtr);
    m_defaultMaterial = std::make_shared<Material>(shaderProgramPtr);
    
    
}



void RefractionApp::InitializeModels()
{
    m_skyboxTexture = TextureCubemapLoader::LoadTextureShared("models/defaultCubemap.png", TextureObject::FormatRGB, TextureObject::InternalFormatSRGB8);

    m_skyboxTexture->Bind();
    float maxLod;
    m_skyboxTexture->GetParameter(TextureObject::ParameterFloat::MaxLod, maxLod);
    TextureCubemapObject::Unbind();
    //m_defaultMaterial->SetUniformValue("Alpha", 0.3f);
    //m_defaultMaterial->SetUniformValue("cubeColor", glm::vec3(0.0f, 0.0f, 1.0f));

    Mesh::SemanticMap semanticMap;
    semanticMap[VertexAttribute::Semantic::Position] = 0; // matches shader location

    //std::shared_ptr<Texture2DObject> planeTexture = Texture2DLoader::LoadTextureShared("noise_maps/iceland_heightmap.png", TextureObject::FormatRGB, TextureObject::InternalFormatSRGBA8);
    /*std::vector<Vertex> vertices;
    float yScale = 64.0f / 256.0f, yShift = 16.0f;
    int rez = 1;
    int height = 1756;
    int width = 2624;
    //unsigned bytePerPixel = nrChannels;
    for(int i = 0; i < height; i++)
    {
        for(int j = 0; j < width; j++)
        {
            unsigned char* pixelOffset = planeTexture + (j + width * i) * 8;
            unsigned char y = pixelOffset[0];

            // vertex
            vertices.push_back( {{-height/2.0f + height*i/(float)height, (int) y * yScale - yShift, -width/2.0f + width*j/(float)width}, {0,0,0}});   // vx

        }
    }*/
    
    std::shared_ptr<Mesh> mesh = CreatePlaneMesh(100, 100, 5.1f);
        // --- Create model ---
    std::shared_ptr<Model> model = std::make_shared<Model>(mesh);


    model->AddMaterial(m_defaultMaterial);

    // --- Scene node ---
    std::shared_ptr<SceneModel> cubeNode =
        std::make_shared<SceneModel>("cube", model);
        
    m_scene.AddSceneNode(cubeNode);
    


}

void RefractionApp::InitializeRenderer()
{
    m_renderer.AddRenderPass(std::make_unique<SkyboxRenderPass>(m_skyboxTexture));
    m_renderer.AddRenderPass(std::make_unique<ForwardRenderPass>());
    glDisable(GL_CULL_FACE);
}

void RefractionApp::RenderGUI()
{

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
            float y = sin(x);
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