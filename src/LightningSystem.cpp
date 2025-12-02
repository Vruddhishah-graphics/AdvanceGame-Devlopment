#include "game/LightningSystem.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstdlib>
#include <cmath>
#include <iostream>

LightningSystem::LightningSystem()
    : vao_(0), vbo_(0), shader_(0),
    uView_(-1), uProj_(-1), uColor_(-1), uAlpha_(-1) {
}

LightningSystem::~LightningSystem() {
    if (vao_) glDeleteVertexArrays(1, &vao_);
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (shader_) glDeleteProgram(shader_);
}

void LightningSystem::Init() {
    InitGL();
}

void LightningSystem::InitGL() {
    // Lightning shader
    const char* vertSrc = R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        
        uniform mat4 uView;
        uniform mat4 uProj;
        
        void main() {
            gl_Position = uProj * uView * vec4(aPos, 1.0);
        }
    )";

    const char* fragSrc = R"(
        #version 330 core
        uniform vec3 uColor;
        uniform float uAlpha;
        
        out vec4 FragColor;
        
        void main() {
            FragColor = vec4(uColor, uAlpha);
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
        std::cerr << "Lightning vertex shader error: " << log << std::endl;
    }

    GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag, 1, &fragSrc, nullptr);
    glCompileShader(frag);

    glGetShaderiv(frag, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(frag, 512, nullptr, log);
        std::cerr << "Lightning fragment shader error: " << log << std::endl;
    }

    shader_ = glCreateProgram();
    glAttachShader(shader_, vert);
    glAttachShader(shader_, frag);
    glLinkProgram(shader_);

    glGetProgramiv(shader_, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(shader_, 512, nullptr, log);
        std::cerr << "Lightning shader link error: " << log << std::endl;
    }

    glDeleteShader(vert);
    glDeleteShader(frag);

    // Get uniform locations
    uView_ = glGetUniformLocation(shader_, "uView");
    uProj_ = glGetUniformLocation(shader_, "uProj");
    uColor_ = glGetUniformLocation(shader_, "uColor");
    uAlpha_ = glGetUniformLocation(shader_, "uAlpha");

    // Create VAO/VBO
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);
}

void LightningSystem::GenerateBolt(LightningBolt& bolt, const glm::vec3& start, const glm::vec3& end, int depth) {
    const int maxDepth = 3;
    const float minSegmentLength = 0.3f;

    bolt.points.clear();
    bolt.points.push_back(start);

    // Recursive midpoint displacement
    std::vector<glm::vec3> current = { start, end };

    for (int d = 0; d < maxDepth; ++d) {
        std::vector<glm::vec3> next;

        for (size_t i = 0; i + 1 < current.size(); ++i) {
            next.push_back(current[i]);

            glm::vec3 mid = (current[i] + current[i + 1]) * 0.5f;

            // Random perpendicular offset
            glm::vec3 dir = glm::normalize(current[i + 1] - current[i]);
            glm::vec3 perp1 = glm::normalize(glm::cross(dir, glm::vec3(0, 1, 0)));
            glm::vec3 perp2 = glm::normalize(glm::cross(dir, perp1));

            float offset = (0.5f - (float)d / maxDepth) * 0.8f;
            float rx = ((float)(std::rand() % 200) / 100.f - 1.f) * offset;
            float ry = ((float)(std::rand() % 200) / 100.f - 1.f) * offset;

            mid += perp1 * rx + perp2 * ry;
            next.push_back(mid);
        }

        next.push_back(current.back());
        current = next;
    }

    bolt.points = current;
    bolt.life = bolt.maxLife;
}

void LightningSystem::TriggerLightning(const glm::vec3& start, const glm::vec3& end) {
    LightningBolt bolt;
    GenerateBolt(bolt, start, end);
    bolts_.push_back(bolt);
}

void LightningSystem::Update(float dt) {
    for (auto it = bolts_.begin(); it != bolts_.end();) {
        it->life -= dt;
        if (it->life <= 0.0f) {
            it = bolts_.erase(it);
        }
        else {
            ++it;
        }
    }
}

void LightningSystem::Render(const glm::mat4& view, const glm::mat4& proj) {
    if (bolts_.empty()) return;

    glUseProgram(shader_);
    glUniformMatrix4fv(uView_, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(uProj_, 1, GL_FALSE, glm::value_ptr(proj));

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending
    glDisable(GL_DEPTH_TEST);
    glLineWidth(3.0f);

    glBindVertexArray(vao_);

    for (const auto& bolt : bolts_) {
        float alpha = bolt.life / bolt.maxLife;

        glUniform3fv(uColor_, 1, glm::value_ptr(bolt.color));
        glUniform1f(uAlpha_, alpha);

        // Upload line strip data
        std::vector<float> data;
        for (const auto& p : bolt.points) {
            data.push_back(p.x);
            data.push_back(p.y);
            data.push_back(p.z);
        }

        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STREAM_DRAW);

        glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)bolt.points.size());
    }

    glLineWidth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindVertexArray(0);
    glUseProgram(0);
}