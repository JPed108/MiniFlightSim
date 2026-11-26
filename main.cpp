#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <string>
#include <utility>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

#include "include/SphereCube.h"
#include "Renderer.h"
#include "FastNoiseLite.h"
#include "include/Camera.h"
#include "ImgIO.h"
#include <ctime>

const float VELOCIDADE = 10.0f;
const int n = 25; // Número de vértices por face do cubo


struct AppState
{
    bool isWireframe = false;
    Camera* camera = nullptr;
};

GLuint compileShader(GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLint logLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        std::string log(logLength, '\0');
        glGetShaderInfoLog(shader, logLength, nullptr, log.data());
        std::cerr << "Shader compilation failed:\n" << log << std::endl;
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

void iniciar()
{
  //define a cor de limpeza 
    glClearColor (0.0, 0.0, 0.0, 0.0);
    glShadeModel (GL_SMOOTH);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0xFFFFFFFF);
    glEnable(GL_DEPTH_TEST);
    
    // Luz
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    // Componentes da luz (RGBA)
    GLfloat ambientLight[]  = { 0.05f, 0.05f, 0.05f, 1.0f }; 
    GLfloat diffuseLight[]  = { 1.00f, 0.98f, 0.92f, 1.0f }; 
    GLfloat specularLight[] = { 0.40f, 0.40f, 0.40f, 1.0f };
    GLfloat emissionMat[] = {1.00f, 0.98f, 0.92f, 1.0f};

    glLightfv(GL_LIGHT0, GL_AMBIENT,  ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
    glMaterialfv(GL_FRONT, GL_EMISSION, emissionMat);

}

void inicializarEsferas(SphereCube spheres[], double r_p, double x, double y, double z, 
                       Shader& shaderPlanet, Shader& shaderStar, std::string heightmap_path)
{
    GLfloat planetColor[3] = {0.0f, 1.0f, 0.0f}; // Verde
    SphereCube planet(250, planetColor, 0.0, 0.0, 0.0, r_p, 0.0, heightmap_path.c_str());
    planet.setShaders(&shaderPlanet);

    //SphereCube planet(200, planetColor, 0.0, 0.0, 0.0, r_p, 0.0, "./heightmap/HeightMap_Marte.png");
    //SphereCube planet(200, planetColor, 0.0, 0.0, 0.0, r_p, 0.0, "./heightmap/lua.png");

    GLfloat starColor[3] = {1.0f, 1.0f, 0.0f}; // Amarelo
    SphereCube star(10, starColor, x, y, z, 1.0, 30.0);
    star.setShaders(&shaderStar);

    spheres[0] = planet;
    spheres[1] = star;
}

void drawAxes(float length) // Gerado por IA para ajudar no debug
{
    // Save current lighting, line width, and color states
    glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT | GL_CURRENT_BIT);

    // Disable lighting so the lines render at full brightness regardless of light angle
    glDisable(GL_LIGHTING);
    glLineWidth(2.5f);

    glBegin(GL_LINES);
        // X-axis (Red)
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(length, 0.0f, 0.0f);

        // Y-axis (Green)
        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, length, 0.0f);

        // Z-axis (Blue)
        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, length);
    glEnd();

    // Restore previous OpenGL states
    glPopAttrib();
}

//função chamada quando a tela é redimensionada 
void redimensionar(GLFWwindow* janela, int largura, int altura )
{
    glViewport (0, 0, (GLsizei) largura, (GLsizei) altura);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity ();

    // https://www.songho.ca/opengl/gl_projectionmatrix.html
    double aspect = (double)largura / (double)altura;
    double fovY = 45.0; // Campo de visão vertical em graus
    double top = 0.1 * tan(fovY * 0.5 * M_PI / 180.0);
    double right = top * aspect;
    glFrustum(-right, right, -top, top, 0.1, 1000.0);
    glMatrixMode(GL_MODELVIEW);
}


//função chamada quando há interação com mouse
void click(GLFWwindow* janela, int botao, int acao, int modificadores )
{
    
}

void generatePlanet(float oceanDepth, float mountainHeight)
{
    int width = 2048;
    int height = 1024;
    std::vector<uint16_t> pixelData(width * height);
    int seed = static_cast<int>(std::time(nullptr));    

    FastNoiseLite continentMask;
    continentMask.SetSeed(seed);
    continentMask.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    continentMask.SetFrequency(0.6f);
    continentMask.SetFractalType(FastNoiseLite::FractalType_FBm);
    continentMask.SetFractalOctaves(5);
    continentMask.SetFractalGain(0.5);

    FastNoiseLite mountainMask;
    mountainMask.SetSeed(seed + 1);
    mountainMask.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    mountainMask.SetFrequency(0.6f);
    mountainMask.SetFractalType(FastNoiseLite::FractalType_Ridged);
    mountainMask.SetFractalOctaves(5);
    mountainMask.SetFractalGain(0.5);

    FastNoiseLite warpNoise;
    warpNoise.SetSeed(seed + 2);
    warpNoise.SetDomainWarpType(FastNoiseLite::DomainWarpType_OpenSimplex2);
    warpNoise.SetFrequency(1.2f);          
    warpNoise.SetDomainWarpAmp(0.45f);    
    warpNoise.SetFractalType(FastNoiseLite::FractalType_DomainWarpProgressive);
    warpNoise.SetFractalOctaves(3);
    warpNoise.SetFractalGain(0.5f);

    #pragma omp parallel for schedule(static)
    for (int j = 0; j < height; j++)
    {
        float v = static_cast<float>(j) / static_cast<float>(height - 1);
        float phi = v * M_PI; // de 0 a PI

        for (int i = 0; i < width; i++)
        {
            float u = static_cast<float>(i) / static_cast<float>(width - 1);
            float theta = u * 2 * M_PI;

            float x = sin(phi) * cos(theta);
            float y = cos(phi);
            float z = sin(phi) * sin(theta);

            warpNoise.DomainWarp(x, y, z);

            float continent = (continentMask.GetNoise(x, y, z) + 1.0f) * 0.5f;

            uint16_t finalHeight = static_cast<uint16_t>(continent * 65535.0f);
            pixelData[j * width + i] = finalHeight;

        }
    }
    
    ImgIO::Image16 image;
    image.width = static_cast<unsigned>(width);
    image.height = static_cast<unsigned>(height);
    image.format = ImgIO::Format::Gray;
    image.pixels = std::move(pixelData);

    std::string error;
    if (ImgIO::savePng("./heightmap/random_map.png", image, error))
    {
        std::cout << "Mapa aleatório gerado!" << std::endl;
    }
    else
    {
        std::cerr << "Falha na gravação do mapa: " << error << std::endl;
    }
    
}

void processarEntrada(GLFWwindow* janela, Camera& camera, float deltaTime)
{
    if (glfwGetKey(janela, GLFW_KEY_RIGHT) == GLFW_PRESS)
        camera.rotateLocal(0.0f, -deltaTime * 50.0f, 0.0f);
    if (glfwGetKey(janela, GLFW_KEY_LEFT) == GLFW_PRESS)
        camera.rotateLocal(0.0f, deltaTime * 50.0f, 0.0f);
    if (glfwGetKey(janela, GLFW_KEY_S) == GLFW_PRESS)
        camera.adjustPitch(-deltaTime * 50.0f);
    if (glfwGetKey(janela, GLFW_KEY_W) == GLFW_PRESS)
        camera.adjustPitch(deltaTime * 50.0f);
    if (glfwGetKey(janela, GLFW_KEY_UP) == GLFW_PRESS)
        camera.changeMaxSpeed(0.01f);
    if (glfwGetKey(janela, GLFW_KEY_DOWN) == GLFW_PRESS)
        camera.changeMaxSpeed(-0.01f);

}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    AppState* appState = static_cast<AppState*>(glfwGetWindowUserPointer(window));
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        appState->isWireframe = !appState->isWireframe;
        //std::cout << "Wireframe mode: " << (appState->isWireframe ? "ON" : "OFF") << std::endl;
        if (appState->isWireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        if (appState->camera->getFlyingStatus() == Camera::CameraStatus::ON_GROUND) 
            appState->camera->changeFlyingStatus(Camera::CameraStatus::TAKING_OFF); // muda o estado da câmera para decolagem

         else if (appState->camera->getFlyingStatus() == Camera::CameraStatus::FLYING || appState->camera->getFlyingStatus() == Camera::CameraStatus::TAKING_OFF)  
            appState->camera->changeFlyingStatus(Camera::CameraStatus::LANDING);    // muda o estado da câmera para pouso
        
    }
}

int main(int argc, char const *argv[])
{
    if (argc < 5) {
        std::cerr << "Uso: " << argv[0] << " <raio_planeta> <x_estrela> <y_estrela> <z_estrela> <opcional: caminho_HeightMap" << std::endl;
        return EXIT_FAILURE;
    }    
    double r_p = std::stod(argv[1]);
    double x = std::stod(argv[2]);
    double y = std::stod(argv[3]);
    double z = std::stod(argv[4]);
    std::string heightmap_path;
    if (argc == 6)
    {
        heightmap_path = argv[5];
    }
    if (argc == 5)
    {
        generatePlanet(0.5f, 0.5f);
        heightmap_path = "./heightmap/random_map.png";
    }
    
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    GLFWwindow* janela = glfwCreateWindow(700, 700, "SuperMiniFlightSim", NULL, NULL );

    if (!janela) {
        glfwTerminate();
        return -1;
    }    
    
    glfwMakeContextCurrent(janela);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwDestroyWindow(janela);
        glfwTerminate();
        return -1;
    }

    {
        Renderer renderer;
        SphereCube spheres[2];
        Shader shaderPlanet("Shaders/planet_vertex.vert", "Shaders/planet_fragment.frag");
        Shader shaderStar("Shaders/general_vertex.vert", "Shaders/general_fragment.frag");
        inicializarEsferas(spheres, r_p, x, y, z, shaderPlanet, shaderStar, heightmap_path);
        Camera camera(&spheres[0], VELOCIDADE); // esferas[0] é o planeta

        AppState AppState;
        AppState.camera = &camera;

        glfwSetWindowUserPointer(janela, &AppState);
        
        //Novidade: funções orientadas a eventos de entrada
        glfwSetFramebufferSizeCallback(janela, redimensionar);
        //glfwSetMouseButtonCallback(janela, clicar);
        glfwSetKeyCallback(janela, key_callback);

        iniciar();

        redimensionar(janela, 700, 700); // Inicializa a projeção

        double lastTime = glfwGetTime();
        while (!glfwWindowShouldClose(janela))
        {
            
            double currentTime = glfwGetTime();
            float deltaTime = static_cast<float>(currentTime - lastTime);
            lastTime = currentTime;
            
            processarEntrada(janela, camera, deltaTime);
            camera.moveForward(deltaTime);
            //std::cout << "Camera Speed: (" << camera.getSpeed() << ")" << std::endl;
            renderer.renderScene(spheres, 2, camera, currentTime);
            
            glfwPollEvents();
        }

        glUseProgram(0);
    }

    glfwDestroyWindow(janela);
    glfwTerminate();

    //generatePlanet(0.1, 0.1);

    return EXIT_SUCCESS;
}
