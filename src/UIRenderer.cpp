#include "game/UIRenderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

namespace game {

    // Simple 5x7 bitmap font patterns for each character
    static const bool FONT_DATA[][35] = {
        // A (0)
        {0,1,1,1,0, 1,0,0,0,1, 1,0,0,0,1, 1,1,1,1,1, 1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1},
        // B (1)
        {1,1,1,1,0, 1,0,0,0,1, 1,0,0,0,1, 1,1,1,1,0, 1,0,0,0,1, 1,0,0,0,1, 1,1,1,1,0},
        // C (2)
        {0,1,1,1,0, 1,0,0,0,1, 1,0,0,0,0, 1,0,0,0,0, 1,0,0,0,0, 1,0,0,0,1, 0,1,1,1,0},
        // D (3)
        {1,1,1,1,0, 1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1, 1,1,1,1,0},
        // E (4)
        {1,1,1,1,1, 1,0,0,0,0, 1,0,0,0,0, 1,1,1,1,0, 1,0,0,0,0, 1,0,0,0,0, 1,1,1,1,1},
        // F (5)
        {1,1,1,1,1, 1,0,0,0,0, 1,0,0,0,0, 1,1,1,1,0, 1,0,0,0,0, 1,0,0,0,0, 1,0,0,0,0},
        // G (6)
        {0,1,1,1,0, 1,0,0,0,1, 1,0,0,0,0, 1,0,1,1,1, 1,0,0,0,1, 1,0,0,0,1, 0,1,1,1,0},
        // H (7)
        {1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1, 1,1,1,1,1, 1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1},
        // I (8)
        {1,1,1,1,1, 0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0, 1,1,1,1,1},
        // J (9)
        {0,0,1,1,1, 0,0,0,1,0, 0,0,0,1,0, 0,0,0,1,0, 0,0,0,1,0, 1,0,0,1,0, 0,1,1,0,0},
        // K (10)
        {1,0,0,0,1, 1,0,0,1,0, 1,0,1,0,0, 1,1,0,0,0, 1,0,1,0,0, 1,0,0,1,0, 1,0,0,0,1},
        // L (11)
        {1,0,0,0,0, 1,0,0,0,0, 1,0,0,0,0, 1,0,0,0,0, 1,0,0,0,0, 1,0,0,0,0, 1,1,1,1,1},
        // M (12)
        {1,0,0,0,1, 1,1,0,1,1, 1,0,1,0,1, 1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1},
        // N (13)
        {1,0,0,0,1, 1,1,0,0,1, 1,0,1,0,1, 1,0,0,1,1, 1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1},
        // O (14)
        {0,1,1,1,0, 1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1, 0,1,1,1,0},
        // P (15)
        {1,1,1,1,0, 1,0,0,0,1, 1,0,0,0,1, 1,1,1,1,0, 1,0,0,0,0, 1,0,0,0,0, 1,0,0,0,0},
        // Q (16)
        {0,1,1,1,0, 1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1, 1,0,1,0,1, 1,0,0,1,0, 0,1,1,0,1},
        // R (17)
        {1,1,1,1,0, 1,0,0,0,1, 1,0,0,0,1, 1,1,1,1,0, 1,0,1,0,0, 1,0,0,1,0, 1,0,0,0,1},
        // S (18)
        {0,1,1,1,1, 1,0,0,0,0, 1,0,0,0,0, 0,1,1,1,0, 0,0,0,0,1, 0,0,0,0,1, 1,1,1,1,0},
        // T (19)
        {1,1,1,1,1, 0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0},
        // U (20)
        {1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1, 0,1,1,1,0},
        // V (21)
        {1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1, 0,1,0,1,0, 0,0,1,0,0},
        // W (22)
        {1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1, 1,0,1,0,1, 1,0,1,0,1, 1,1,0,1,1, 1,0,0,0,1},
        // X (23)
        {1,0,0,0,1, 1,0,0,0,1, 0,1,0,1,0, 0,0,1,0,0, 0,1,0,1,0, 1,0,0,0,1, 1,0,0,0,1},
        // Y (24)
        {1,0,0,0,1, 1,0,0,0,1, 0,1,0,1,0, 0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0},
        // Z (25)
        {1,1,1,1,1, 0,0,0,0,1, 0,0,0,1,0, 0,0,1,0,0, 0,1,0,0,0, 1,0,0,0,0, 1,1,1,1,1},
        // 0 (26)
        {0,1,1,1,0, 1,0,0,0,1, 1,0,0,1,1, 1,0,1,0,1, 1,1,0,0,1, 1,0,0,0,1, 0,1,1,1,0},
        // 1 (27)
        {0,0,1,0,0, 0,1,1,0,0, 0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0, 0,1,1,1,0},
        // 2 (28)
        {0,1,1,1,0, 1,0,0,0,1, 0,0,0,0,1, 0,0,0,1,0, 0,0,1,0,0, 0,1,0,0,0, 1,1,1,1,1},
        // 3 (29)
        {1,1,1,1,1, 0,0,0,0,1, 0,0,0,1,0, 0,0,1,1,0, 0,0,0,0,1, 1,0,0,0,1, 0,1,1,1,0},
        // 4 (30)
{0,0,0,1,0, 0,0,1,1,0, 0,1,0,1,0, 1,0,0,1,0, 1,1,1,1,1, 0,0,0,1,0, 0,0,0,1,0},
// 5 (31)
{1,1,1,1,1, 1,0,0,0,0, 1,1,1,1,0, 0,0,0,0,1, 0,0,0,0,1, 1,0,0,0,1, 0,1,1,1,0},
// 6 (32)
{0,1,1,1,0, 1,0,0,0,0, 1,0,0,0,0, 1,1,1,1,0, 1,0,0,0,1, 1,0,0,0,1, 0,1,1,1,0},
// 7 (33)
{1,1,1,1,1, 0,0,0,0,1, 0,0,0,1,0, 0,0,1,0,0, 0,1,0,0,0, 0,1,0,0,0, 0,1,0,0,0},
// 8 (34)
{0,1,1,1,0, 1,0,0,0,1, 1,0,0,0,1, 0,1,1,1,0, 1,0,0,0,1, 1,0,0,0,1, 0,1,1,1,0},
// 9 (35)
{0,1,1,1,0, 1,0,0,0,1, 1,0,0,0,1, 0,1,1,1,1, 0,0,0,0,1, 0,0,0,0,1, 0,1,1,1,0}, };

    // Map characters to font indices
    static int GetFontIndex(char c) {
        if (c >= 'A' && c <= 'Z') return c - 'A';          // 0-25
        if (c >= 'a' && c <= 'z') return c - 'a';          // Use uppercase
        if (c >= '0' && c <= '9') return 26 + (c - '0');   // 26-35
        if (c == ' ') return -1; // Space (don't render)
        return -1; // Unknown character (don't render)
    }    UIRenderer::UIRenderer()
        : shader_(0), vao_(0), vbo_(0),
        uProjection_(-1), uModel_(-1), uColor_(-1),
        screenWidth_(1280), screenHeight_(720) {
    }

    UIRenderer::~UIRenderer() {
        if (vao_) glDeleteVertexArrays(1, &vao_);
        if (vbo_) glDeleteBuffers(1, &vbo_);
        if (shader_) glDeleteProgram(shader_);
    }

    void UIRenderer::Init(int screenWidth, int screenHeight) {
        screenWidth_ = screenWidth;
        screenHeight_ = screenHeight;

        InitShaders();
        InitMesh();
        InitFont();
        SetScreenSize(screenWidth, screenHeight);

        std::cout << "  UI Renderer initialized with text support\n";
    }

    void UIRenderer::InitShaders() {
        const char* vertSrc = R"(
        #version 330 core
        layout(location = 0) in vec2 aPos;
        
        uniform mat4 uProjection;
        uniform mat4 uModel;
        
        void main() {
            gl_Position = uProjection * uModel * vec4(aPos, 0.0, 1.0);
        }
    )";

        const char* fragSrc = R"(
        #version 330 core
        uniform vec4 uColor;
        out vec4 FragColor;
        
        void main() {
            FragColor = uColor;
        }
    )";

        GLuint vert = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vert, 1, &vertSrc, nullptr);
        glCompileShader(vert);

        GLint success;
        glGetShaderiv(vert, GL_COMPILE_STATUS, &success);
        if (!success) {
            char log[512];
            glGetShaderInfoLog(vert, 512, nullptr, log);
            std::cerr << "UI vertex shader error: " << log << std::endl;
        }

        GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(frag, 1, &fragSrc, nullptr);
        glCompileShader(frag);

        glGetShaderiv(frag, GL_COMPILE_STATUS, &success);
        if (!success) {
            char log[512];
            glGetShaderInfoLog(frag, 512, nullptr, log);
            std::cerr << "UI fragment shader error: " << log << std::endl;
        }

        shader_ = glCreateProgram();
        glAttachShader(shader_, vert);
        glAttachShader(shader_, frag);
        glLinkProgram(shader_);

        glGetProgramiv(shader_, GL_LINK_STATUS, &success);
        if (!success) {
            char log[512];
            glGetProgramInfoLog(shader_, 512, nullptr, log);
            std::cerr << "UI shader link error: " << log << std::endl;
        }

        glDeleteShader(vert);
        glDeleteShader(frag);

        uProjection_ = glGetUniformLocation(shader_, "uProjection");
        uModel_ = glGetUniformLocation(shader_, "uModel");
        uColor_ = glGetUniformLocation(shader_, "uColor");
    }

    void UIRenderer::InitMesh() {
        float vertices[] = {
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f
        };

        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);

        glBindVertexArray(vao_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

        glBindVertexArray(0);
    }

    void UIRenderer::InitFont() {
        // Font data is already defined above
    }

    void UIRenderer::SetScreenSize(int width, int height) {
        screenWidth_ = width;
        screenHeight_ = height;
        projection_ = glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);
    }

    void UIRenderer::BeginUI() {
        glUseProgram(shader_);
        glUniformMatrix4fv(uProjection_, 1, GL_FALSE, glm::value_ptr(projection_));

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void UIRenderer::EndUI() {
        glEnable(GL_DEPTH_TEST);
        glUseProgram(0);
    }

    void UIRenderer::RenderRect(float x, float y, float width, float height,
        const glm::vec4& color) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(x, y, 0.0f));
        model = glm::scale(model, glm::vec3(width, height, 1.0f));

        glUniformMatrix4fv(uModel_, 1, GL_FALSE, glm::value_ptr(model));
        glUniform4fv(uColor_, 1, glm::value_ptr(color));

        glBindVertexArray(vao_);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindVertexArray(0);
    }

    void UIRenderer::RenderBorder(float x, float y, float width, float height,
        float thickness, const glm::vec4& color) {
        RenderRect(x, y, width, thickness, color);
        RenderRect(x, y + height - thickness, width, thickness, color);
        RenderRect(x, y, thickness, height, color);
        RenderRect(x + width - thickness, y, thickness, height, color);
    }

    void UIRenderer::RenderHealthBar(float x, float y, float width, float height,
        float percentage, const glm::vec3& color) {
        percentage = glm::clamp(percentage, 0.0f, 1.0f);

        RenderRect(x, y, width, height, glm::vec4(0.2f, 0.2f, 0.2f, 0.8f));

        if (percentage > 0.0f) {
            glm::vec4 fillColor(color.r, color.g, color.b, 0.9f);
            RenderRect(x + 2, y + 2, (width - 4) * percentage, height - 4, fillColor);
        }

        RenderBorder(x, y, width, height, 2.0f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    }

    void UIRenderer::RenderChar(char c, float x, float y, float scale, const glm::vec4& color) {
        int idx = GetFontIndex(c);
        if (idx < 0 || idx >= 36) return; // Fixed: array has 36 elements (0-35)

        const bool* pattern = FONT_DATA[idx];
        float pixelSize = scale;

        // Draw 5x7 grid
        for (int row = 0; row < 7; row++) {
            for (int col = 0; col < 5; col++) {
                if (pattern[row * 5 + col]) {
                    RenderRect(x + col * pixelSize, y + row * pixelSize,
                        pixelSize, pixelSize, color);
                }
            }
        }
    }

    void UIRenderer::RenderText(const std::string& text, float x, float y,
        float scale, const glm::vec3& color) {
        float charWidth = 5.0f * scale;
        float spacing = 2.0f * scale;
        glm::vec4 col4(color.r, color.g, color.b, 1.0f);

        float currentX = x;
        for (char c : text) {
            RenderChar(c, currentX, y, scale, col4);
            currentX += charWidth + spacing;
        }
    }

    void UIRenderer::RenderCenteredText(const std::string& text, float y,
        float scale, const glm::vec3& color) {
        float charWidth = 5.0f * scale;
        float spacing = 2.0f * scale;
        float totalWidth = text.length() * (charWidth + spacing) - spacing;
        float x = (screenWidth_ - totalWidth) / 2.0f;

        RenderText(text, x, y, scale, color);
    }

} // namespace game