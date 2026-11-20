#include "Renderer.h"

void Renderer::updateStarLight(const SphereCube &star, double time)
{
    glPushMatrix();
        if (star.planetData.orbitalPeriod > 1E-3)
        {
            double dTheta = 2.0*M_PI/star.planetData.orbitalPeriod;
            glRotated(time * glm::degrees(dTheta), 0.0, 1.0, 0.0);
        }
        const glm::vec3 starPos = star.planetData.coords;
        glTranslatef(starPos.x, starPos.y, starPos.z);

        GLfloat lightPosition[] = {0.0f, 0.0f, 0.0f, 1.0f};
        glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glPopMatrix();
}

void Renderer::setEmissiveMaterial(bool isStar)
{
    if (isStar)
    {
        GLfloat emission[] = {1.0f, 0.92f, 0.55f, 1.0f};
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

    updateStarLight(spheres[1], time);

    for (int i = 0; i < n; i++)
    {
        glPushMatrix();
            if (std::abs(spheres[i].planetData.orbitalPeriod) > 1E-3)
            {
                double dTheta = 2.0 * M_PI / spheres[i].planetData.orbitalPeriod;
                glRotated(time * glm::degrees(dTheta), 0.0, 1.0, 0.0);
            }

            const glm::vec3 pos = spheres[i].planetData.coords;
            const float r = spheres[i].planetData.r;

            glTranslatef(pos.x, pos.y, pos.z);
            glScalef(r, r, r);

            setEmissiveMaterial(i == 1);
            spheres[i].draw();
        glPopMatrix();
    }
    setEmissiveMaterial(false);
    glfwSwapBuffers(glfwGetCurrentContext());
    

}
