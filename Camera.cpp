#include "Camera.hpp"

namespace gps {
    bool firstMouse = true;
    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        //TODO
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = glm::normalize(glm::cross(Camera::cameraFrontDirection, Camera::cameraRightDirection));
        this->cameraRightDirection = glm::normalize(glm::cross(glm::vec3(0.0f, 0.0f, -1.0f), cameraUp));
        this->cameraFrontDirection = glm::normalize(-this->cameraPosition + this->cameraTarget);
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        //TODO
        return glm::lookAt(this->cameraPosition, this->cameraPosition + this->cameraFrontDirection, glm::vec3(0.0, 1.0, 0.0));
    }

    glm::vec3 Camera::getCameraTarget()
    {
        return cameraTarget;
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        //TODO
        switch (direction)
        {
        case MOVE_FORWARD: this->cameraPosition += this->cameraFrontDirection * speed;
            break;
        case MOVE_BACKWARD: this->cameraPosition -= this->cameraFrontDirection * speed;
            break;
        case MOVE_LEFT: this->cameraPosition -= this->cameraRightDirection * speed;
            break;
        case MOVE_RIGHT: this->cameraPosition += this->cameraRightDirection * speed;
            break;
        default:;
        }

    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        //TODO
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        Camera::cameraFrontDirection = glm::normalize(front);
        Camera::cameraRightDirection = glm::normalize(glm::cross(Camera::cameraFrontDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
        Camera::cameraUpDirection = glm::normalize(glm::cross(Camera::cameraRightDirection, Camera::cameraFrontDirection));
    }

    //Camera setCamera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {

    //}

}
