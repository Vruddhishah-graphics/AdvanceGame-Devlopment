#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/quaternion.hpp>
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include "game/glm_minimal.h"

namespace game {

    class Node {
    public:
        Node(const std::string& name) : name_(name) {}

        void SetPosition(const glm::vec3& pos) { position_ = pos; }

        void SetRotation(float angleDeg, const glm::vec3& axis) {
            // Store as axis-angle instead of quaternion
            rotationAngle_ = angleDeg;
            rotationAxis_ = axis;
        }

        void SetScale(const glm::vec3& scale) { scale_ = scale; }

        glm::vec3 GetPosition() const { return position_; }
        glm::vec3 GetScale() const { return scale_; }

        void AddChild(std::shared_ptr<Node> child) {
            children_.push_back(child);
            child->parent_ = this;
        }

        void SetRenderCallback(std::function<void(const glm::mat4&)> callback) {
            renderCallback_ = callback;
        }

        void Render(const glm::mat4& parentTransform = glm::mat4(1.0f)) {
            glm::mat4 localTransform = parentTransform * GetLocalTransform();

            if (renderCallback_) {
                renderCallback_(localTransform);
            }

            for (auto& child : children_) {
                child->Render(localTransform);
            }
        }

        void Update(float dt) {
            if (updateCallback_) {
                updateCallback_(dt);
            }

            for (auto& child : children_) {
                child->Update(dt);
            }
        }

        void SetUpdateCallback(std::function<void(float)> callback) {
            updateCallback_ = callback;
        }

        std::string GetName() const { return name_; }

    private:
        glm::mat4 GetLocalTransform() const {
            glm::mat4 T = glm::translate(glm::mat4(1.0f), position_);
            glm::mat4 R = glm::rotate(glm::mat4(1.0f),
                glm::radians(rotationAngle_),
                rotationAxis_);
            glm::mat4 S = glm::scale(glm::mat4(1.0f), scale_);
            return T * R * S;
        }

        std::string name_;
        glm::vec3 position_{ 0.0f };
        float rotationAngle_{ 0.0f };
        glm::vec3 rotationAxis_{ 0.0f, 1.0f, 0.0f };
        glm::vec3 scale_{ 1.0f };

        Node* parent_ = nullptr;
        std::vector<std::shared_ptr<Node>> children_;

        std::function<void(const glm::mat4&)> renderCallback_;
        std::function<void(float)> updateCallback_;
    };
}
 // namespace game