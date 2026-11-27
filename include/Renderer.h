
#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include "SphereCube.h"
#include "Camera.h"

class Renderer
{
private:
    void updateStarLight(const glm::vec3& sunWorldPosition);
    void setEmissiveMaterial(bool isStar);

public:
    void renderScene(SphereCube *spheres, int n, Camera& camera, double time);



};