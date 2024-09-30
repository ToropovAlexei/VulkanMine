#include "glm/fwd.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
  Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f),
         glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = -90.0f,
         float pitch = 0.0f, float fov = 45.0f, float aspectRatio = 4.0f / 3.0f,
         float nearPlane = 0.1f, float farPlane = 100.0f)
      : m_position(position), m_worldUp(up), m_yaw(yaw), m_pitch(pitch),
        m_fov(fov), m_aspectRatio(aspectRatio), m_nearPlane(nearPlane),
        m_farPlane(farPlane) {
    updateCameraVectors();
  }

  glm::mat4 getViewMatrix() const {
    return glm::lookAt(m_position, m_position + m_front, m_up);
  }
  glm::mat4 getProjectionMatrix() const {
    auto projection = glm::perspective(glm::radians(m_fov), m_aspectRatio,
                                       m_nearPlane, m_farPlane);
    projection[1][1] *= -1; // Инвертируем ось Y для Vulkan
    return projection;
  }
  void setProjection(float newFov, float newAspectRatio, float newNearPlane,
                     float newFarPlane) {
    m_fov = newFov;
    m_aspectRatio = newAspectRatio;
    m_nearPlane = newNearPlane;
    m_farPlane = newFarPlane;
  }
  void setPosition(const glm::vec3 &newPosition) { m_position = newPosition; }
  glm::vec3 getPosition() const { return m_position; }
  void setOrientation(float newYaw, float newPitch) {
    m_yaw = newYaw;
    m_pitch = newPitch;
    updateCameraVectors();
  }
  float getYaw() const { return m_yaw; }
  float getPitch() const { return m_pitch; }
  glm::vec3 getFront() const { return m_front; }
  glm::vec3 getRight() const { return m_right; }
  glm::vec3 getUp() const { return m_up; }

  void move(const glm::vec3 &direction) { m_position += direction; }

  void rotate(float yawOffset, float pitchOffset, bool constrainPitch = true) {
    m_yaw += yawOffset;
    m_pitch += pitchOffset;

    if (constrainPitch) {
      if (m_pitch > 89.0f)
        m_pitch = 89.0f;
      if (m_pitch < -89.0f)
        m_pitch = -89.0f;
    }

    updateCameraVectors();
  }

private:
  glm::vec3 m_position;
  glm::vec3 m_front;
  glm::vec3 m_up;
  glm::vec3 m_right;
  glm::vec3 m_worldUp;

  float m_yaw;
  float m_pitch;

  float m_fov;
  float m_aspectRatio;
  float m_nearPlane;
  float m_farPlane;

  void updateCameraVectors() {
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    newFront.y = sin(glm::radians(m_pitch));
    newFront.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_front = glm::normalize(newFront);

    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_front));
  }
};