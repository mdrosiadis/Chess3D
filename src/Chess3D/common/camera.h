#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glfw3.h>
#include <glm/glm.hpp>

class Camera {
public:
    GLFWwindow* window;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    // Initial position : on +Z
    glm::vec3 position;
    glm::vec3 lookTo;
    // Initial horizontal angle : toward -Z
    float horizontalAngle;
    // Initial vertical angle : none
    float verticalAngle;
    // Field of View
    float FoV;
    float speed; // units / second
    float mouseSpeed;
    float fovSpeed;

    Camera() = default;
    Camera(GLFWwindow* window);
    void update();
    glm::vec3 getLookDirection();
};

#endif
