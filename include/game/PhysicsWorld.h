#pragma once
#include <btBulletDynamicsCommon.h>
#include <glm/glm.hpp>
#include <vector>
#include <memory>

namespace game {

    class PhysicsWorld {
    public:
        PhysicsWorld();
        ~PhysicsWorld();

        void StepSimulation(float dt);

        // Rigid body creation
        btRigidBody* CreateBox(const glm::vec3& halfExtents,
            const glm::vec3& position,
            float mass);

        btRigidBody* CreateSphere(float radius,
            const glm::vec3& position,
            float mass);

        btRigidBody* CreateCapsule(float radius, float height,
            const glm::vec3& position,
            float mass);

        // Static objects
        btRigidBody* CreateStaticBox(const glm::vec3& halfExtents,
            const glm::vec3& position);

        // Character controller
        btKinematicCharacterController* CreateCharacter(
            float radius, float height,
            const glm::vec3& position);

        // Utilities
        glm::mat4 GetTransform(btRigidBody* body);
        void SetTransform(btRigidBody* body, const glm::mat4& transform);

    private:
        std::unique_ptr<btDefaultCollisionConfiguration> collisionConfig_;
        std::unique_ptr<btCollisionDispatcher> dispatcher_;
        std::unique_ptr<btBroadphaseInterface> broadphase_;
        std::unique_ptr<btSequentialImpulseConstraintSolver> solver_;
        std::unique_ptr<btDiscreteDynamicsWorld> dynamicsWorld_;

        std::vector<std::unique_ptr<btCollisionShape>> shapes_;
        std::vector<std::unique_ptr<btRigidBody>> bodies_;
    };

} // namespace game