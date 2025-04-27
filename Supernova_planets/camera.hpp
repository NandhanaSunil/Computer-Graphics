#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "config.hpp" 

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};


class Camera {
public:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
   
    float Yaw;
    float Pitch;
    float MovementSpeed;
    
    Camera(glm::vec3 position = INITIAL_CAMERA_POS, glm::vec3 up = WORLD_UP, glm::vec3 front = INITIAL_CAMERA_FRONT);

    
    glm::mat4 GetViewMatrix() const;

    void ProcessKeyboard(Camera_Movement direction, float deltaTime);

    void updateRightAndUp();

private:
};

#endif // CAMERA_H