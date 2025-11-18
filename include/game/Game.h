#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <string>
#include <memory>

#include "game/Camera.h"
#include "game/MeshUtils.h"
#include "game/Node.h"
#include "game/Texture.h"

namespace game {

    struct AABB {
        glm::vec3 min;
        glm::vec3 max;
    };

    struct Entity {
        glm::vec3 pos{ 0.f };
        glm::vec3 size{ 1.f };
        glm::vec3 color{ 1.f };
        bool dynamic = false;

        AABB bounds() const {
            glm::vec3 r = size * 0.5f;
            return { pos - r, pos + r };
        }
    };

    struct Player {
        glm::vec3 pos{ 0.f };
        glm::vec3 color{ 1.f };
        glm::vec3 size{ 1.f };
        float speed = 4.0f;

        AABB bounds() const {
            glm::vec3 r = size * 0.5f;
            return { pos - r, pos + r };
        }
    };

    struct Cheese {
        glm::vec3 pos;
        bool taken = false;
    };

    struct PowerUp {
        glm::vec3 pos;
        int type = 0;       // 0 = shield, 1 = speed, 2 = freeze
        bool taken = false;
        float rotation = 0.f;
    };

    struct Particle {
        glm::vec3 pos;
        glm::vec3 vel;
        glm::vec3 color;
        float life;
        float size;
    };

    class Game {
    public:
        void Run();

    private:
        // ============================================================
        // SYSTEM SETUP
        // ============================================================
        void initWindow();
        void initGL();
        void initShaders();
        void initMeshes();
        void initTextures();
        void initSceneGraph();
        void resetWorld();
        void loop();
        void update(float dt);
        void updateCannon(float dt);
        void render();
        void printInstructions();

        // ============================================================
        // INPUT HANDLING
        // ============================================================
        static bool keys_[1024];
        static void keyCb(GLFWwindow*, int key, int scancode, int action, int mods);

        // ============================================================
        // UTILITY
        // ============================================================
        GLuint compile(GLenum type, const std::string& src);
        GLuint link(GLuint v, GLuint f);
        std::string loadText(const std::string& path);

        static bool intersects(const AABB& a, const AABB& b);
        static glm::vec3 overlapVec(const AABB& a, const AABB& b);

        // ============================================================
        // PARTICLE SYSTEM
        // ============================================================
        void spawnParticles(const glm::vec3& pos, const glm::vec3& color, int count);

        // ============================================================
        // TEXT RENDERING (STUB)
        // ============================================================
        void drawText(float x, float y, const std::string& txt,
            float r = 1.f, float g = 1.f, float b = 1.f);

        // ============================================================
        // WINDOW DATA
        // ============================================================
        GLFWwindow* win_ = nullptr;
        int width_ = 1280;
        int height_ = 720;

        // ============================================================
        // CAMERA
        // ============================================================
        Camera cam_;
        float cameraAngle_ = 0.0f;
        float cameraHeight_ = 18.0f;
        float cameraDistance_ = 20.0f;

        // ============================================================
        // SHADERS & UNIFORMS
        // ============================================================
        GLuint prog_ = 0;

        GLint uModel_ = -1;
        GLint uView_ = -1;
        GLint uProj_ = -1;
        GLint uViewPos_ = -1;

        GLint uBaseColor_ = -1;
        GLint uEmissive_ = -1;

        GLint uKa_ = -1;
        GLint uKd_ = -1;
        GLint uKs_ = -1;
        GLint uShine_ = -1;

        GLint uL1pos_ = -1;
        GLint uL1col_ = -1;
        GLint uL2pos_ = -1;
        GLint uL2col_ = -1;

        GLint uUseTexture_ = -1;
        GLint uTexture_ = -1;

        // ============================================================
        // MESHES
        // ============================================================
        Mesh box_;
        Mesh sphere_;
        Mesh cone_;
        Mesh cyl_;

        // --- OBJ MODELS ---
        Mesh mouseModel_;
        Mesh catModel_;
        Mesh cheeseModel_;

        // ============================================================
        // TEXTURES
        // ============================================================
        std::unique_ptr<Texture> grassTex_;
        std::unique_ptr<Texture> stoneTex_;
        std::unique_ptr<Texture> metalTex_;
        std::unique_ptr<Texture> woodTex_;

        // ============================================================
        // SCENE GRAPH
        // ============================================================
        std::shared_ptr<Node> sceneRoot_;
        std::shared_ptr<Node> cannonRoot_;
        std::shared_ptr<Node> cannonBarrel_;
        float cannonRotation_ = 0.0f;
        float cannonTilt_ = 0.0f;

        // ============================================================
        // GAME ENTITIES
        // ============================================================
        Player mouse_;
        Player cat_;

        std::vector<Entity> walls_;
        std::vector<Entity> furniture_;
        std::vector<Cheese> cheeses_;
        std::vector<PowerUp> powerups_;
        std::vector<Particle> particles_;

        int collected_ = 0;
        int level_ = 1;

        // Mouse win/lose state
        bool gameOver_ = false;
        bool mouseWin_ = false;

        // ============================================================
        // POWER-UP STATE
        // ============================================================
        bool mouseInvincible_ = false;
        bool mouseSpeedBoost_ = false;
        bool catFrozen_ = false;

        float powerUpTimer_ = 0.f;
        int currentPowerUp_ = -1;
    };

} // namespace game