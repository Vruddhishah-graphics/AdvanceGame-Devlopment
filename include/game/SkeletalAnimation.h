#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>
#include <string>
#include <map>

namespace game {

    struct Bone {
        std::string name;
        int id;
        glm::mat4 offsetMatrix;
        std::vector<int> children;
    };

    struct KeyFrame {
        glm::vec3 position;
        glm::quat rotation;
        glm::vec3 scale;
        float timeStamp;
    };

    struct BoneAnimation {
        std::vector<KeyFrame> keyFrames;

        glm::mat4 Interpolate(float time);
    };

    class Animation {
    public:
        std::string name;
        float duration;
        float ticksPerSecond;
        std::map<std::string, BoneAnimation> boneAnimations;

        void Update(float dt, std::vector<glm::mat4>& boneTransforms,
            const std::vector<Bone>& skeleton);
    };

    class AnimatedModel {
    public:
        std::vector<Bone> skeleton;
        std::map<std::string, Animation> animations;
        std::vector<glm::mat4> boneTransforms;

        Animation* currentAnimation = nullptr;
        float currentTime = 0.0f;
        bool looping = true;

        bool LoadFromFile(const std::string& path);
        void PlayAnimation(const std::string& name);
        void Update(float dt);
        void SetBoneUniforms(GLuint shader);
    };

} // namespace game