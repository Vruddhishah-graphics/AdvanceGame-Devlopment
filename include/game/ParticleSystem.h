#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <memory>

class ParticleSystem {
public:
    ParticleSystem(int maxParticles = 2000);
    ~ParticleSystem();

    void Init();
    void Update(float dt);
    void Render(const glm::mat4& view, const glm::mat4& proj, const glm::vec3& camPos);

    void CreateExplosion(const glm::vec3& position, const glm::vec4& color, int count = 30);
    void CreateTrail(const glm::vec3& position, const glm::vec4& color);

private:
    struct Particle {
        glm::vec3 position;
        glm::vec3 velocity;
        glm::vec4 color;
        float life;
        float size;

        Particle() : position(0), velocity(0), color(1), life(0), size(0.1f) {}
    };

    void InitGL();
    int FindUnusedParticle();

    std::vector<Particle> particles_;
    int maxParticles_;
    int lastUsedParticle_;

    GLuint vao_;
    GLuint vbo_;
    GLuint shader_;

    GLint uView_;
    GLint uProj_;
    GLint uCamPos_;
};