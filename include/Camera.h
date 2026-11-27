#pragma once
#include <glad/glad.h>
#include <cmath>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "SphereCube.h"

class Camera
{    

public:
    enum class CameraStatus {
                ON_GROUND, 
                TAKING_OFF, 
                FLYING, 
                LANDING 
            }; // Estados da câmera'

private:
    glm::vec3 position;
    glm::quat orientation;
    glm::quat baseOrientation;
    float speed = 0.0f;                 // Velocidade de movimento da câmera
    float takeOffSpeed = 2.0f;          // Velocidade de decolagem da câmera
    float maxSpeed = 5.0f;              // Velocidade máxima de vôo da câmera
    float minSpeed = 2.5f;              // Velocidade mínima de vôo da câmera
    float acceleration = 0.5f;          // Aceleração da câmera na decolagem e pouso
    float minHeight = 0.2f;            // Altura mínima da câmera sobre o planeta 
    float maxHeight = 8.0f;             // Altura máxima da câmera sobre o planeta
    float height = 0.01f;               // Altura atual da câmera sobre o planeta
    SphereCube* planet = nullptr;       // Ponteiro para o planeta
    float pitchAngle = -20.0f;
    CameraStatus status = CameraStatus::ON_GROUND;

public:
    Camera(SphereCube* planet = nullptr, float speed = 1.0f);
    glm::vec3 getPosition() const { return position; }
    glm::quat getOrientation() const { return orientation; }
    glm::mat4 getViewMatrix();
    glm::vec3 getForwardVector() const;
    glm::vec3 getRightVector() const;
    glm::vec3 getUpVector() const;
    glm::quat viewDirection(glm::quat baseOrientation) const;
    CameraStatus getFlyingStatus(void) { return status;};

    void setPosition(const glm::vec3& pos) { position = pos; }
    void setOrientation(const glm::quat& orient) { orientation = orient; }
    float getSpeed() const { return speed; }
    void rotateLocal(float pitchDegrees, float yawDegrees, float rollDegrees); // rotação local da câmera
    void adjustPitch(float deltaPitch);
    void adjustYawn(float deltaYawn);
    void moveForward(float dt);
    void takeOff(float dt);
    void landing(float dt);
    void changeFlyingStatus(CameraStatus newStatus) { status = newStatus; };
    void changeMaxSpeed(float dv);
    void updateSpeed(float dt);
    
    
};
