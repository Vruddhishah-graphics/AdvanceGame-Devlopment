#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

class LightningSystem {
public:
    LightningSystem();
    ~LightningSystem();

    void Init();
    void Update(float dt);
    void Render(const glm::mat4& view, const glm::mat4& proj);

    void TriggerLightning(const glm::vec3& start, const glm::vec3& end);

private:
    struct LightningBolt {
        std::vector<glm::vec3> points;
        float life;
        float maxLife;
        glm::vec3 color;

        LightningBolt() : life(0), maxLife(0.3f), color(0.8f, 0.9f, 1.0f) {}
    };

    void GenerateBolt(LightningBolt& bolt, const glm::vec3& start, const glm::vec3& end, int depth = 0);
    void InitGL();

    std::vector<LightningBolt> bolts_;

    GLuint vao_;
    GLuint vbo_;
    GLuint shader_;

    GLint uView_;
    GLint uProj_;
    GLint uColor_;
    GLint uAlpha_;
};