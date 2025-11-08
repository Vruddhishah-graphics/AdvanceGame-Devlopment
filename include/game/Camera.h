#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace game {

class Camera {
public:
    void setProjection(float fovDeg, float aspect, float nearZ, float farZ) {
        proj_ = glm::perspective(glm::radians(fovDeg), aspect, nearZ, farZ);
    }
    void setPosition(const glm::vec3& p) { pos_ = p; }
    void setTarget(const glm::vec3& t)   { target_ = t; }

    void updateTracking(const glm::vec3& p1, const glm::vec3& p2, float smooth = 0.12f) {
        glm::vec3 mid = 0.5f * (p1 + p2);
        glm::vec3 desired = mid + glm::vec3(0.f, 6.5f, 12.f);
        pos_    = glm::mix(pos_, desired, smooth);
        target_ = glm::mix(target_, mid, smooth);
    }

    glm::mat4 view() const { return glm::lookAt(pos_, target_, up_); }
    const glm::mat4& proj() const { return proj_; }
    const glm::vec3& position() const { return pos_; }

private:
    glm::vec3 pos_{ 0.f, 6.f, 12.f };
    glm::vec3 target_{ 0.f, 0.f, 0.f };
    glm::vec3 up_{ 0.f, 1.f, 0.f };
    glm::mat4 proj_{ 1.f };
};

} // namespace game
