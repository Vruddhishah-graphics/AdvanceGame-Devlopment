#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace game {

    class Camera {
    public:
        Camera()
            : pos_(0.f, 25.f, 5.f),
            target_(0.f, 0.f, 0.f),
            up_(0.f, 1.f, 0.f),
            proj_(1.f) {
        }

        void setProjection(float fovDeg, float aspect, float nearZ, float farZ) {
            proj_ = glm::perspective(glm::radians(fovDeg), aspect, nearZ, farZ);
        }

        void setPosition(const glm::vec3& p) { pos_ = p; }
        void setTarget(const glm::vec3& t) { target_ = t; }

        glm::vec3 position() const { return pos_; }
        glm::mat4 view()     const { return glm::lookAt(pos_, target_, up_); }
        glm::mat4 proj()     const { return proj_; }

        // Smooth hybrid top-down follow
        void updateTracking(const glm::vec3& p1, const glm::vec3& p2, float smooth = 0.1f) {
            glm::vec3 mid = 0.5f * (p1 + p2);
            glm::vec3 desired = mid + glm::vec3(0.f, 22.f, 6.f);
            pos_ = glm::mix(pos_, desired, smooth);
            target_ = glm::mix(target_, mid, smooth);
        }

    private:
        glm::vec3 pos_;
        glm::vec3 target_;
        glm::vec3 up_;
        glm::mat4 proj_;
    };

} // namespace game
