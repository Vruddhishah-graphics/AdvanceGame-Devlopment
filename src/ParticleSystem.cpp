#include "game/ParticleSystem.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstdlib>
#include <cmath>
#include <iostream>

ParticleSystem::ParticleSystem(int maxParticles)
    : maxParticles_(maxParticles), lastUsedParticle_(0),
    vao_(0), vbo_(0), shader_(0),
    uView_(-1), uProj_(-1), uCamPos_(-1) {
    particles_.resize(maxParticles);
}

ParticleSystem::~ParticleSystem() {
    if (vao_) glDeleteVertexArrays(1, &vao_);
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (shader_) glDeleteProgram(shader_);
}

void ParticleSystem::Init() {
    InitGL();
}

void ParticleSystem::InitGL() {
    // Simple particle shader
    const char* vertSrc = R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in vec4 aColor;
        layout(location = 2) in float aSize;
        
        uniform mat4 uView;
        uniform mat4 uProj;
        uniform vec3 uCamPos;
        
        out vec4 vColor;
        
        void main() {
            vColor = aColor;
            vec4 worldPos = vec4(aPos, 1.0);
            vec4 viewPos = uView * worldPos;
            gl_Position = uProj * viewPos;
            
            // Billboard size calculation
            float dist = length(viewPos.xyz);
            gl_PointSize = max(1.0, aSize * 500.0 / dist);
        }
    )";

    const char* fragSrc = R"(
        #version 330 core
        in vec4 vColor;
        out vec4 FragColor;
        
        void main() {
            // Round particle shape
            vec2 coord = gl_PointCoord - vec2(0.5);
            float dist = length(coord);
            if (dist > 0.5) discard;
            
            float alpha = vColor.a * (1.0 - dist * 2.0);
            FragColor = vec4(vColor.rgb, alpha);
        }
    )";

    // Compile shaders
    GLuint vert = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert, 1, &vertSrc, nullptr);
    glCompileShader(vert);

    GLint success;
    glGetShaderiv(vert, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(vert, 512, nullptr, log);
        std::cerr << "Particle vertex shader error: " << log << std::endl;
    }

    GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag, 1, &fragSrc, nullptr);
    glCompileShader(frag);

    glGetShaderiv(frag, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(frag, 512, nullptr, log);
        std::cerr << "Particle fragment shader error: " << log << std::endl;
    }

    shader_ = glCreateProgram();
    glAttachShader(shader_, vert);
    glAttachShader(shader_, frag);
    glLinkProgram(shader_);

    glGetProgramiv(shader_, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(shader_, 512, nullptr, log);
        std::cerr << "Particle shader link error: " << log << std::endl;
    }

    glDeleteShader(vert);
    glDeleteShader(frag);

    // Get uniform locations
    uView_ = glGetUniformLocation(shader_, "uView");
    uProj_ = glGetUniformLocation(shader_, "uProj");
    uCamPos_ = glGetUniformLocation(shader_, "uCamPos");

    // Create VAO/VBO
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);

    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)0);

    // Color
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(3 * sizeof(float)));

    // Size
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(7 * sizeof(float)));

    glBindVertexArray(0);
}

int ParticleSystem::FindUnusedParticle() {
    // Search from last used particle
    for (int i = lastUsedParticle_; i < maxParticles_; ++i) {
        if (particles_[i].life <= 0.0f) {
            lastUsedParticle_ = i;
            return i;
        }
    }

    // Search from beginning
    for (int i = 0; i < lastUsedParticle_; ++i) {
        if (particles_[i].life <= 0.0f) {
            lastUsedParticle_ = i;
            return i;
        }
    }

    // Override oldest
    lastUsedParticle_ = 0;
    return 0;
}

void ParticleSystem::CreateExplosion(const glm::vec3& position, const glm::vec4& color, int count) {
    for (int i = 0; i < count; ++i) {
        int idx = FindUnusedParticle();

        // Random direction
        float angle = (float)(std::rand() % 360) * 3.14159265f / 180.f;
        float elevation = (float)(std::rand() % 180 - 90) * 3.14159265f / 180.f;
        float speed = 2.f + (float)(std::rand() % 100) / 50.f;

        glm::vec3 dir(
            std::cos(elevation) * std::cos(angle),
            std::sin(elevation),
            std::cos(elevation) * std::sin(angle)
        );

        particles_[idx].position = position;
        particles_[idx].velocity = dir * speed;
        particles_[idx].color = color;
        particles_[idx].life = 0.8f + (float)(std::rand() % 100) / 200.f;
        particles_[idx].size = 0.15f + (float)(std::rand() % 100) / 1000.f;
    }
}

void ParticleSystem::CreateTrail(const glm::vec3& position, const glm::vec4& color) {
    int idx = FindUnusedParticle();

    particles_[idx].position = position;
    particles_[idx].velocity = glm::vec3(0, 0.5f, 0);
    particles_[idx].color = color;
    particles_[idx].life = 0.3f;
    particles_[idx].size = 0.1f;
}

void ParticleSystem::Update(float dt) {
    for (auto& p : particles_) {
        if (p.life > 0.0f) {
            p.life -= dt;
            p.position += p.velocity * dt;
            p.velocity.y -= 9.8f * dt; // Gravity

            // Fade out
            p.color.a = p.life;
        }
    }
}

void ParticleSystem::Render(const glm::mat4& view, const glm::mat4& proj, const glm::vec3& camPos) {
    glUseProgram(shader_);
    glUniformMatrix4fv(uView_, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(uProj_, 1, GL_FALSE, glm::value_ptr(proj));
    glUniform3fv(uCamPos_, 1, glm::value_ptr(camPos));

    // Prepare particle data
    std::vector<float> data;
    data.reserve(maxParticles_ * 8);

    int count = 0;
    for (const auto& p : particles_) {
        if (p.life > 0.0f) {
            data.push_back(p.position.x);
            data.push_back(p.position.y);
            data.push_back(p.position.z);
            data.push_back(p.color.r);
            data.push_back(p.color.g);
            data.push_back(p.color.b);
            data.push_back(p.color.a);
            data.push_back(p.size);
            count++;
        }
    }

    if (count > 0) {
        glEnable(GL_PROGRAM_POINT_SIZE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending
        glDepthMask(GL_FALSE);

        glBindVertexArray(vao_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STREAM_DRAW);

        glDrawArrays(GL_POINTS, 0, count);

        glDepthMask(GL_TRUE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBindVertexArray(0);
    }

    glUseProgram(0);
}