//
// Created by Kristoffer Borgstrøm on 21/03/2023.
//
#include <memory>

#include <ituGL/camera/Camera.h>
#include <ituGL/application/Application.h>
#include <ituGL/application/Window.h>

class Controller {
public:
    Controller(glm::vec3 position = glm::vec3(0.0f, 30.0f, 30.0f), float rotationSpeed = 0.5f, float translationSpeed = 30.0f);

    void Initialize(Window& window);
    void UpdateCamera(Window& window, float time);

    glm::mat4 GetViewProjectionMatrix();
    glm::vec3 GetCameraPosition();

private:
    Camera m_camera;

    glm::vec3 m_camPosition;
    glm::vec2 m_mousePosition;
    float m_rotationSpeed;
    float m_translationSpeed;
    bool m_cameraEnabled = false;
    bool m_spacePressed = false;

    // Orientation
    float m_yaw;
    float m_pitch;
    float m_roll;
    glm::vec3 m_up;
};