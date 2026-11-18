#include "Camera.h"
#include <iostream>

Camera::Camera(SphereCube* planet, float speed)
    : maxSpeed(speed), height(minHeight), planet(planet)
     {
       const glm::vec3 initialNormal(1.0f, 0.0f, 0.0f);
      const double initialTerrainHeight = height < maxHeight
          ? planet->localHeight(initialNormal)
          : 0.0;
       const float initialRadius = static_cast<float>(
          planet->planetData.r + height + initialTerrainHeight);
       position = planet->planetData.coords + initialNormal * initialRadius;
        
        glm::vec3 defaultUp(0.0f, 1.0f, 0.0f);
        glm::vec3 surfaceNormal(1.0f, 0.0f, 0.0f);

        baseOrientation = glm::rotation(defaultUp, surfaceNormal);
        orientation = viewDirection(baseOrientation);
        
        
     }

glm::mat4 Camera::getViewMatrix()
{
    glm::mat4 rotationMatrix = glm::mat4_cast(glm::conjugate(orientation));

    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), -position);

    return rotationMatrix * translationMatrix;
}

void Camera::rotateLocal(float pitchDegrees, float yawDegrees, float rollDegrees)
{
    // Rotação local da câmera usando quaternions. 
    glm::quat pitch = glm::angleAxis(glm::radians(pitchDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::quat yaw = glm::angleAxis(glm::radians(yawDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::quat roll = glm::angleAxis(glm::radians(rollDegrees), glm::vec3(0.0f, 0.0f, 1.0f));

    // orientation * pitch * yaw * roll aplica a rotação local da câmera, enquanto pitch * yaw * roll * orientation aplicaria a rotação global. 
    baseOrientation = glm::normalize(baseOrientation * pitch * yaw * roll);
    orientation = viewDirection(baseOrientation);
}

void Camera::adjustPitch(float deltaPitch)
{
    pitchAngle += deltaPitch;
    pitchAngle = glm::clamp(pitchAngle, -89.0f, 89.0f); // Limita o ângulo de pitch para evitar inversões
    orientation = viewDirection(baseOrientation);
}

void Camera::adjustYawn(float deltaYawn)
{
    
}

glm::vec3 Camera::getForwardVector() const
{
    return baseOrientation * glm::vec3(0.0f, 0.0f, -1.0f);
}

glm::vec3 Camera::getRightVector() const
{
    return baseOrientation * glm::vec3(1.0f, 0.0f, 0.0f);
}

glm::vec3 Camera::getUpVector() const
{
    return baseOrientation * glm::vec3(0.0f, 1.0f, 0.0f);
}

glm::quat Camera::viewDirection(glm::quat baseOrientation) const
{
    glm::quat lookAngle = glm::angleAxis(glm::radians(pitchAngle), glm::vec3(1.0f, 0.0f, 0.0f));
    return glm::normalize(baseOrientation * lookAngle);
}

void Camera::moveForward(float dt)
{
    updateSpeed(dt);
    if (std::abs(speed) < 0.001f) return;
    glm::vec3 planetCenter(planet->planetData.coords);

    position += getForwardVector() * speed * dt;

    glm::vec3 surfaceNormal = glm::normalize(position - planetCenter);
    const double terrainHeight = height < maxHeight
        ? planet->localHeight(surfaceNormal)
        : 0.0;
    const float totalRadius = static_cast<float>(
        planet->planetData.r + height + terrainHeight);
    position = planetCenter + surfaceNormal * totalRadius;

    glm::vec3 currentUp = getUpVector();
    glm::quat alignmentRot = glm::rotation(currentUp, surfaceNormal);
    baseOrientation = glm::normalize(alignmentRot * baseOrientation);
    orientation = viewDirection(baseOrientation);

}



void Camera::takeOff(float dt)
{
    if (speed < maxSpeed && height < maxHeight) 
    {
        speed += 1.5*acceleration * dt;
        if (speed > maxSpeed) speed = maxSpeed;
    }

    if (speed > takeOffSpeed && height < maxHeight)
    {
        height += 0.2*speed * dt;
        if (height > maxHeight) height = maxHeight;
    }
    if (height >= maxHeight) 
    {
        changeFlyingStatus(CameraStatus::FLYING);
    }
}

void Camera::landing(float dt)
{
    if (speed > 0.0f) 
    {
        if (height > minHeight) // Desaceleração normal quando a câmera está no ar
        {
            if (speed >takeOffSpeed)
                speed -= acceleration * dt;
        }
        else // Desaceleração mais rápida quando a câmera está no solo
        {
            speed -= 4.0 * acceleration * dt;
        }
        if (speed < 0.0f) speed = 0.0f;
    }

    if (height > minHeight) 
    {
        height -= 0.4*speed * dt; 
        if (height < minHeight) 
        {
            height = minHeight;
        }
    }

    if (height <= minHeight && speed <= 0.0f) 
    {
        changeFlyingStatus(CameraStatus::ON_GROUND);
    }
}

void Camera::changeMaxSpeed(float dv)
{
    if (status == CameraStatus::FLYING) // Só altera a velocidade máxima de vôo se a câmera estiver voando
    {
        maxSpeed += dv;
        speed = maxSpeed;
        if (maxSpeed < minSpeed) changeFlyingStatus(CameraStatus::LANDING); // Se a velocidade máxima for menor que a mínima, faz a câmera pousar
        //std::cout << "Max Speed changed to: " << maxSpeed << std::endl;
    } 
    if (maxSpeed < takeOffSpeed) maxSpeed = takeOffSpeed + 0.01f; // Garante que a velocidade máxima é ligeiramente maior que a velocidade de decolagem

}

void Camera::updateSpeed(float dt)
{
    if (status == CameraStatus::TAKING_OFF) 
        takeOff(dt);
    else if (status == CameraStatus::LANDING)
        landing(dt);
    /*else if (status == CameraStatus::FLYING) 
    {
        if (speed < maxSpeed) 
        {
            speed += acceleration * dt;
            if (speed > maxSpeed) speed = maxSpeed;
        }
    }*/
}