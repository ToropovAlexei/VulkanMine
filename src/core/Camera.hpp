#include "Frustum.hpp"
#include "glm/fwd.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
  Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
         glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = -90.0f,
         float pitch = 0.0f, float fov = 45.0f, float aspectRatio = 4.0f / 3.0f,
         float nearPlane = 0.1f, float farPlane = 100.0f)
      : m_position(position), m_worldUp(up), m_yaw(yaw), m_pitch(pitch),
        m_fov(fov), m_aspectRatio(aspectRatio), m_nearPlane(nearPlane),
        m_farPlane(farPlane) {
    updateCameraVectors();
    updateFrustum();
  }

  inline glm::mat4 getViewMatrix() const noexcept {
    return glm::lookAt(m_position, m_position + m_front, m_up);
  }
  inline glm::mat4 getProjectionMatrix() const noexcept {
    auto projection = glm::perspective(glm::radians(m_fov), m_aspectRatio,
                                       m_nearPlane, m_farPlane);
    projection[1][1] *= -1; // Инвертируем ось Y
    return projection;
  }
  inline Frustum &getFrustum() noexcept { return m_frustum; }
  inline void setProjection(float newFov, float newAspectRatio,
                            float newNearPlane, float newFarPlane) noexcept {
    m_fov = newFov;
    m_aspectRatio = newAspectRatio;
    m_nearPlane = newNearPlane;
    m_farPlane = newFarPlane;
    updateFrustum();
  }
  inline void setPosition(const glm::vec3 &newPosition) noexcept {
    m_position = newPosition;
    updateFrustum();
  }
  inline glm::vec3 &getPosition() noexcept { return m_position; }
  inline void setOrientation(float newYaw, float newPitch) noexcept {
    m_yaw = newYaw;
    m_pitch = newPitch;
    updateCameraVectors();
    updateFrustum();
  }
  inline float getYaw() const noexcept { return m_yaw; }
  inline float getPitch() const noexcept { return m_pitch; }
  inline glm::vec3 &getFront() noexcept { return m_front; }
  inline glm::vec3 &getRight() noexcept { return m_right; }
  inline glm::vec3 &getUp() noexcept { return m_up; }

  inline void move(const glm::vec3 &direction) noexcept {
    m_position += direction;
    updateFrustum();
  }

  inline void rotate(float yawOffset, float pitchOffset,
                     bool constrainPitch = true) noexcept {
    m_yaw += yawOffset;
    m_pitch += pitchOffset;

    if (constrainPitch) {
      if (m_pitch > 89.0f)
        m_pitch = 89.0f;
      if (m_pitch < -89.0f)
        m_pitch = -89.0f;
    }

    updateCameraVectors();
    updateFrustum();
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

  Frustum m_frustum;

  inline void updateCameraVectors() noexcept {
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    newFront.y = sin(glm::radians(m_pitch));
    newFront.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_front = glm::normalize(newFront);

    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_front));
  }

  inline void updateFrustum() noexcept {
    m_frustum.extractPlanes(getProjectionMatrix() * getViewMatrix());
  }
};