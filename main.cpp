#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "include/SphereCube.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include "include/Camera.h"
#include <glm/gtc/type_ptr.hpp>

const float VELOCIDADE = 10.0f;
const int n = 25; // Número de vértices por face do cubo


struct AppState
{
    bool isWireframe = false;
    Camera* camera = nullptr;
};

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
    GLfloat ambientLight[]  = { 0.3f, 0.3f, 0.30f, 1.0f }; 
    GLfloat diffuseLight[]  = { 1.00f, 0.98f, 0.92f, 1.0f }; 
    GLfloat specularLight[] = { 0.40f, 0.40f, 0.40f, 1.0f };

    glLightfv(GL_LIGHT0, GL_AMBIENT,  ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
}

void inicializarEsferas(SphereCube spheres[], double r_p, double x, double y, double z)
{
    GLfloat planetColor[3] = {0.0f, 1.0f, 0.0f}; // Verde
    SphereCube planet(200, planetColor, 0.0, 0.0, 0.0, r_p, 0.0, "./heightmap/EarthHeightMap.jpg");
    //SphereCube planet(200, planetColor, 0.0, 0.0, 0.0, r_p, 0.0, "./heightmap/HeightMap_Marte.jpg");
    //SphereCube planet(200, planetColor, 0.0, 0.0, 0.0, r_p, 0.0, "./heightmap/lua.jpg");

    GLfloat starColor[3] = {1.0f, 1.0f, 0.0f}; // Amarelo
    SphereCube star(10, starColor, x, y, z, 1.0, 30.0);

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

void desenhar_esferas(SphereCube *esferas, int n, Camera& camera, double dt)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glMultMatrixf(glm::value_ptr(camera.getViewMatrix()));

    const glm::vec3 starPos = esferas[1].planetData.coords;    

    // drawAxes(esferas[0].planetData.r * 1.5f); // Desenhar eixos para debug
    
    double time = glfwGetTime();
    glPushMatrix();
        if (esferas[1].planetData.orbitalPeriod > 1E-3)
        {
            double dTheta = 2.0 * M_PI / esferas[1].planetData.orbitalPeriod;
            glRotated(-time * glm::degrees(dTheta), 0.0, 1.0, 0.0);
        }

        glTranslatef(
            esferas[1].planetData.coords.x,
            esferas[1].planetData.coords.y,
            esferas[1].planetData.coords.z
        );
        GLfloat lightPosition[] = {0.0f, 0.0f, 0.0f, 1.0f};
        glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    glPopMatrix();

    // Desenhar as n esferas no array spheres
    for (int i = 0; i < n; i++)
    {
        GLdouble *sphere_vertices = esferas[i].getVertices();
        GLdouble *sphere_colors = esferas[i].getColors();
        GLdouble *sphere_normals = esferas[i].getNormals();
        GLuint *sphere_indices = esferas[i].getIndices();

        
        glPushMatrix();
        if (esferas[i].planetData.orbitalPeriod > 1E-3)
        {
            double dTheta = 2.0 * M_PI / esferas[i].planetData.orbitalPeriod;
            glRotated(-time * glm::degrees(dTheta), 0.0, 1.0, 0.0);
        }
        glTranslatef(esferas[i].planetData.coords.x, esferas[i].planetData.coords.y, esferas[i].planetData.coords.z);
        //glRotatef(time * 5.0, 0.0, 1.0, 0.0); // Rotação da esfera em torno de seu próprio eixo
        glScalef(esferas[i].planetData.r, esferas[i].planetData.r, esferas[i].planetData.r);
        glPushAttrib(GL_CURRENT_BIT);
            //ativa arrays que serão usados
            glEnableClientState(GL_COLOR_ARRAY);
            glEnableClientState(GL_VERTEX_ARRAY);

            //associa dados aos arrays
            glColorPointer(3, GL_DOUBLE, 0, sphere_colors);
            glVertexPointer(3, GL_DOUBLE, 0, sphere_vertices);

            if (sphere_normals)
            {
                glEnableClientState(GL_NORMAL_ARRAY);
                glNormalPointer(GL_DOUBLE, 0, sphere_normals);
            }



            const GLsizei sphere_index_count = esferas[i].getIndicesCount();
            glDrawElements(GL_TRIANGLE_STRIP, sphere_index_count, GL_UNSIGNED_INT, sphere_indices);
            glPopAttrib();
        glPopMatrix();
    }

    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);

    glfwSwapBuffers(glfwGetCurrentContext());
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

void processarEntrada(GLFWwindow* janela, Camera& camera, float deltaTime)
{
    //if (glfwGetKey(janela, GLFW_KEY_W) == GLFW_PRESS)
    //    camera.moveForward(deltaTime);
    //if (glfwGetKey(janela, GLFW_KEY_S) == GLFW_PRESS)
    //    camera.moveForward(-deltaTime);
    //if (glfwGetKey(janela, GLFW_KEY_A) == GLFW_PRESS)
    //    camera.rotateLocal(0.0, 0.0f, deltaTime * 50.0f);
    //if (glfwGetKey(janela, GLFW_KEY_D) == GLFW_PRESS)
    //    camera.rotateLocal(0.0f, 0.0f, -deltaTime * 50.0f);

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

         else if (appState->camera->getFlyingStatus() == Camera::CameraStatus::FLYING) 
            appState->camera->changeFlyingStatus(Camera::CameraStatus::LANDING);    // muda o estado da câmera para pouso
        
    }
}

int main(int argc, char const *argv[])
{
    if (argc != 5) {
        std::cerr << "Uso: " << argv[0] << " <raio_planeta> <x_estrela> <y_estrela> <z_estrela>" << std::endl;
        return EXIT_FAILURE;
    }    
    double r_p = std::stod(argv[1]);
    double x = std::stod(argv[2]);
    double y = std::stod(argv[3]);
    double z = std::stod(argv[4]);

    SphereCube esferas[2];
    inicializarEsferas(esferas, r_p, x, y, z);
    Camera camera(&esferas[0], VELOCIDADE); // esferas[0] é o planeta

    AppState AppState;
    AppState.camera = &camera;
    if (!glfwInit())
        return -1;

    GLFWwindow* janela = glfwCreateWindow(700, 700, "SuperMiniFlightSim", NULL, NULL );

    if (!janela) {
        glfwTerminate();
        return -1;
    }    
    
    glfwMakeContextCurrent(janela);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glfwSetWindowUserPointer(janela, &AppState);
    
    //Novidade: funções orientadas a eventos de entrada
    glfwSetFramebufferSizeCallback(janela, redimensionar);
    //glfwSetMouseButtonCallback(janela, clicar);
    glfwSetKeyCallback(janela, key_callback);

    iniciar();

    redimensionar(janela, 700, 700); // Inicializa a projeção

    double lastFrame = glfwGetTime();
    while (!glfwWindowShouldClose(janela))
    {
        glfwPollEvents();

        double currentFrame = glfwGetTime();
        float deltaTime = static_cast<float>(currentFrame - lastFrame);
        lastFrame = currentFrame;

        processarEntrada(janela, camera, deltaTime);
        camera.moveForward(deltaTime);
        //std::cout << "Camera Speed: (" << camera.getSpeed() << ")" << std::endl;
        
        desenhar_esferas(esferas, 2, camera, deltaTime);
    }

    glfwDestroyWindow(janela);
    glfwTerminate();

    return EXIT_SUCCESS;
}
