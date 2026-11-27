#include "Renderer.h"
#include <glm/gtc/matrix_transform.hpp>

void Renderer::updateStarLight(const glm::vec3& sunWorldPosition)
{
    glPushMatrix();
        glTranslatef(sunWorldPosition.x, sunWorldPosition.y, sunWorldPosition.z);

        GLfloat lightPosition[] = {0.0f, 0.0f, 0.0f, 1.0f};
        glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glPopMatrix();
}

void Renderer::setEmissiveMaterial(bool isStar)
{
    if (isStar)
    {
        GLfloat emission[] = {1.0f, 1.0f, 1.0f, 1.0f};
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
    }
    else
    {
        GLfloat emission[] = {0.0f, 0.0f, 0.0, 1.0f};
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
    }
}

void Renderer::renderScene(SphereCube *spheres, int n, Camera &camera, double time)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMultMatrixf(glm::value_ptr(camera.getViewMatrix()));

    const glm::vec3 planetCenter = spheres[1].planetData.coords;
    const glm::vec3 sunPosition = spheres[2].planetData.coords;
    const double sunOrbitalPeriod = spheres[2].planetData.orbitalPeriod;
    glm::vec3 sunWorldPosition = sunPosition;

    if (std::abs(sunOrbitalPeriod) > 1E-3)
    {
        // Calculamos o ângulo que a estrela percorre em uma unidade de tempo
        const float angle = static_cast<float>(time * 2.0 * M_PI / sunOrbitalPeriod);
        // Primeiro criamos a matriz de rotação em torno do eixo y 
        const glm::mat4 orbit = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
        // transladamos e rotacionamos a estrela
        sunWorldPosition = planetCenter + glm::vec3(orbit * glm::vec4(sunPosition - planetCenter, 0.0f));

    }

    updateStarLight(sunWorldPosition); 
   
    
    for (int i = 0; i < n; i++)
    {
        glPushMatrix();
            // Como rotacionamos a estrela manualmente, pulamos a rotação dela
            if (i != 2 && std::abs(spheres[i].planetData.orbitalPeriod) > 1E-3)
            {
                double dTheta = 2.0 * M_PI / spheres[i].planetData.orbitalPeriod;
                glRotated(time * glm::degrees(dTheta), 0.0, 1.0, 0.0);
            }
            
            // Se i == 2, estamos desenhando a estrela
            // Nesse caso, transladamos a estrela pela posição calculada manualmente
            const glm::vec3 pos = (i == 2) ? sunWorldPosition : spheres[i].planetData.coords;
            const float r = spheres[i].planetData.r;

            glTranslatef(pos.x, pos.y, pos.z);
            glScalef(r, r, r);
            if (i == 0)
            {
                Shader* skyShader = spheres[0].getShader();
                skyShader->use();
                skyShader->setVec3("uCameraPos", glm::vec3(0.0f)); // Câmera na origem no espaço da câmera
                skyShader->setVec3("uPlanetCenter", glm::vec3(camera.getViewMatrix() * glm::vec4(planetCenter, 1.0)));
                skyShader->setVec3("uSunPos", glm::vec3(camera.getViewMatrix() * glm::vec4(sunWorldPosition, 1.0)));
                skyShader->setFloat("uPlanetRadius", spheres[1].planetData.r);
            }

            setEmissiveMaterial(i == 2);
            spheres[i].draw(time);
        glPopMatrix();
    }
    setEmissiveMaterial(false);
    glfwSwapBuffers(glfwGetCurrentContext());
    

}
