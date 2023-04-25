//
// Created by Kristoffer Borgstrøm on 21/03/2023.
//

#include "Controller.hpp"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

Controller::Controller(glm::vec3 position, float rotationSpeed, float translationSpeed) :
    m_camPosition(position),
    m_rotationSpeed(rotationSpeed),
    m_translationSpeed(translationSpeed)
{
}

void Controller::Initialize(Window &window)
{
    m_camera.SetViewMatrix(m_camPosition, glm::vec3(0.0f));
    float aspectRatio = window.GetAspectRatio();
    m_camera.SetPerspectiveProjectionMatrix(1.0f, aspectRatio, 0.1f, 1000.0f);
    m_mousePosition = window.GetMousePosition(true);
}

void Controller::UpdateCamera(Window& window, float deltaTime)
{

    // Update if camera is enabled (controlled by SPACE key)
    {
        bool enablePressed = window.IsKeyPressed(GLFW_KEY_SPACE);
        if (enablePressed && !m_spacePressed)
        {
            m_cameraEnabled = !m_cameraEnabled;

            window.SetMouseVisible(!m_cameraEnabled);
            m_mousePosition = window.GetMousePosition(true);
        }
        m_spacePressed = enablePressed;
    }

    if (!m_cameraEnabled)
        return;

    glm::mat4 viewTransposedMatrix = glm::transpose(m_camera.GetViewMatrix());
    glm::vec3 viewRight = viewTransposedMatrix[0];
    glm::vec3 viewForward = -viewTransposedMatrix[2];

    // Update camera translation
    {
        glm::vec2 inputTranslation(0.0f);

        if (window.IsKeyPressed(GLFW_KEY_A)) {
            inputTranslation.x += -1.0f;
        }
        if (window.IsKeyPressed(GLFW_KEY_D)) {
            inputTranslation.x += 1.0f;
        }
        if (window.IsKeyPressed(GLFW_KEY_W)) {
            inputTranslation.y += 1.0f;
        }
        if (window.IsKeyPressed(GLFW_KEY_S)) {
            inputTranslation.y += -1.0f;
        }

        inputTranslation *= m_translationSpeed * deltaTime;

        // Double speed if SHIFT is pressed
        if (window.IsKeyPressed(GLFW_KEY_LEFT_SHIFT))
            inputTranslation *= 2.0f;

        m_camPosition += inputTranslation.x * viewRight + inputTranslation.y * viewForward;
    }

    // Update camera rotation
    {
        glm::vec2 mousePosition = window.GetMousePosition(true);
        glm::vec2 deltaMousePosition = mousePosition - m_mousePosition;
        m_mousePosition = mousePosition;

        glm::vec3 inputRotation(-deltaMousePosition.x, deltaMousePosition.y, 0.0f);

        inputRotation *= m_rotationSpeed;

        viewForward = glm::rotate(inputRotation.x, glm::vec3(0,1,0)) * glm::rotate(inputRotation.y, glm::vec3(viewRight)) * glm::vec4(viewForward, 0);
    }

    // Update view matrix
    m_camera.SetViewMatrix(m_camPosition, m_camPosition + viewForward);
}

glm::mat4 Controller::GetViewProjectionMatrix() {
    return m_camera.GetViewProjectionMatrix();
}

glm::vec3 Controller::GetCameraPosition() {
    return m_camPosition;
}