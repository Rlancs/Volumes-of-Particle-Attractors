#pragma once
#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Directions of camera movement
enum Camera_Movement {
    Forward,
    Backward,
    Left,
    Right
};

// Camera config
const float yaw = -90.0f;
const float pitch = 0.0f;
const float speed = 2.5f;
const float sensitivity = 0.1f;
const float zoom = 45.0f;

class Camera
{
public:

    // Camera attributes
    glm::vec3 Position;
    glm::vec3 FrontSide;
    glm::vec3 Up;
    glm::vec3 RightSide;
    glm::vec3 WorldUp;

    float Yaw;
    float Pitch;

    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    /// <summary>
    /// Constructor using vectors
    /// </summary>
    /// <param name="position"></param>
    /// <param name="up"></param>
    /// <param name="yaw"></param>
    /// <param name="pitch"></param>
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = yaw, float pitch = pitch) : FrontSide(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(speed), MouseSensitivity(sensitivity), Zoom(zoom)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    /// <summary>
    /// Constructor using scalar values (float)
    /// </summary>
    /// <param name="posX"></param>
    /// <param name="posY"></param>
    /// <param name="posZ"></param>
    /// <param name="upX"></param>
    /// <param name="upY"></param>
    /// <param name="upZ"></param>
    /// <param name="yaw"></param>
    /// <param name="pitch"></param>
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : FrontSide(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(speed), MouseSensitivity(sensitivity), Zoom(zoom)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    /// <summary>
    /// View matrix of the camera
    /// </summary>
    /// <returns></returns>
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + FrontSide, Up);
    }

    /// <summary>
    /// Processes keyboard input for camera movement
    /// </summary>
    /// <param name="direction"></param>
    /// <param name="deltaTime"></param>
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        if (direction == Forward)
            Position += FrontSide * velocity;
        if (direction == Backward)
            Position -= FrontSide * velocity;
        if (direction == Left)
            Position -= RightSide * velocity;
        if (direction == Right)
            Position += RightSide * velocity;
    }

    /// <summary>
    /// Processes mouse input for camera rotation
    /// </summary>
    /// <param name="xoffset"></param>
    /// <param name="yoffset"></param>
    /// <param name="constrainPitch"></param>
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        // Stops screen flipping
        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        updateCameraVectors();
    }

    /// <summary>
    /// Processes scroll wheel input for camera zoom
    /// </summary>
    /// <param name="yoffset"></param>
    void ProcessMouseScroll(float yoffset)
    {
        Zoom -= (float)yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 45.0f)
            Zoom = 45.0f;
    }

private:

    /// <summary>
    /// Calculates the front, up and right side vectors of the camera
    /// </summary>
    void updateCameraVectors()
    {
        // Calculate the front vector
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        FrontSide = glm::normalize(front);
        // Re-calculate the up and right vectors
        RightSide = glm::normalize(glm::cross(FrontSide, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up = glm::normalize(glm::cross(RightSide, FrontSide));
    }
};
#endif