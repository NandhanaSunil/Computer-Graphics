#include "camera.hpp"
#include <glm/glm.hpp>

Camera::Camera(glm::vec3 position, glm::vec3 up, glm::vec3 front) :
    Position(position),
    WorldUp(up),
    Front(front),
    MovementSpeed(CAMERA_SPEED)
    // other members will be Yaw, Pitch, Zoom , can be used later
{   
    // right and up vectors for camera
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up    = glm::normalize(glm::cross(Right, Front));
}

glm::mat4 Camera::GetViewMatrix() const {
    //this is for getting view 
    return glm::lookAt(Position, Position + Front, Up);
}
void Camera::UpdateCameraVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);

    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up    = glm::normalize(glm::cross(Right, Front));
}

void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime) {
    float velocity = MovementSpeed * deltaTime;
    if (direction == FORWARD)
        Position += Front * velocity;
    if (direction == BACKWARD)
        Position -= Front * velocity;
    if (direction == LEFT)
        Position -= Right * velocity;
    if (direction == RIGHT)
        Position += Right * velocity;
    if (direction == UP)
        Position += WorldUp * velocity; 
    if (direction == DOWN)
        Position -= WorldUp * velocity; 

   
}


void Camera::updateRightAndUp() {  
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}