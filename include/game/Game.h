#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <memory>

#include "game/Camera.h"
#include "game/MeshUtils.h"
#include "game/Node.h"
#include "game/Texture.h"
#include "game/ParticleSystem.h"
#include "game/LightningSystem.h"
#include "game/UIRenderer.h"
#include "game/SoundSystem.h"



namespace game {

    enum class GameState {
        MENU,
        PLAYING,
        PAUSED,
        LEVEL_TRANSITION,
        MOUSE_WIN,
        CAT_WIN,
        GAME_OVER
    };

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
        float rotation = 0.0f;
        int lives = 3;
        float invulnerabilityTimer = 0.0f;

        AABB bounds() const {
            glm::vec3 r = size * 0.5f;
            return { pos - r, pos + r };
        }
    };

    struct Cheese {
        glm::vec3 pos;
        bool taken = false;
        float rotation = 0.0f;
        float bobOffset = 0.0f;
    };

    struct PowerUp {
        glm::vec3 pos;
        int type = 0; // 0=shield, 1=speed, 2=freeze
        bool taken = false;
        float rotation = 0.f;
        float bobOffset = 0.0f;
        float lifetime = 15.0f;
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
        // Initialization
        void initWindow();
        void initGL();
        void initShaders();
        void initMeshes();
        void initTextures();
        void initSceneGraph();


        // Game flow
        void resetWorld();
        void startGame();
        void nextLevel();
        void loop();

        // Update systems
        void update(float dt);
        void updateMenu(float dt);
        void updatePlaying(float dt);
        void updatePaused(float dt);
        void updateCannon(float dt);
        void updateAI(float dt);
        void updatePowerUps(float dt);
        void updatePhysics(float dt);

        // Rendering
        void render();
        void renderScene();
        void renderUI();
        void renderMenu();
        void renderPauseMenu();
        void renderGameOver();

        // Game mechanics
        void checkCollisions();
        void checkWinConditions();
        void spawnParticles(const glm::vec3& pos, const glm::vec3& color, int count);
        void spawnPowerUp();
        void applyPowerUp(int type);
        void loseLife();

        // Utilities
        void printInstructions();
        static bool keys_[1024];
        static void keyCb(GLFWwindow*, int key, int, int action, int);
        GLuint compile(GLenum type, const std::string& src);
        GLuint link(GLuint v, GLuint f);
        std::string loadText(const std::string& path);
        static bool intersects(const AABB& a, const AABB& b);
        static glm::vec3 overlapVec(const AABB& a, const AABB& b);
        std::unique_ptr<SoundSystem> soundSystem_;
        bool soundEnabled_ = true;


        // Window
        GLFWwindow* win_ = nullptr;
        int width_ = 1600;
        int height_ = 900;

        // Camera
        Camera cam_;
        float cameraAngle_ = 0.0f;
        float cameraHeight_ = 20.0f;
        float cameraDistance_ = 25.0f;

        // OpenGL resources
        GLuint prog_ = 0;
        GLint uModel_ = -1, uView_ = -1, uProj_ = -1;
        GLint uViewPos_ = -1, uBaseColor_ = -1, uEmissive_ = -1;
        GLint uKa_ = -1, uKd_ = -1, uKs_ = -1, uShine_ = -1;
        GLint uL1pos_ = -1, uL1col_ = -1;
        GLint uL2pos_ = -1, uL2col_ = -1;
        GLint uUseTexture_ = -1, uTexture_ = -1;

        // Meshes
        Mesh box_, sphere_, cone_, cyl_;
        Mesh mouseModel_, catModel_, cheeseModel_;

        // Textures
        std::unique_ptr<Texture> grassTex_;
        std::unique_ptr<Texture> stoneTex_;
        std::unique_ptr<Texture> metalTex_;
        std::unique_ptr<Texture> woodTex_;

        // Scene graph
        std::shared_ptr<Node> sceneRoot_;
        std::shared_ptr<Node> cannonRoot_;
        std::shared_ptr<Node> cannonBarrel_;
        float cannonRotation_ = 0.0f;
        float cannonTilt_ = 0.0f;

        // Game entities
        Player mouse_;
        Player cat_;
        std::vector<Entity> walls_;
        std::vector<Entity> furniture_;
        std::vector<Cheese> cheeses_;
        std::vector<PowerUp> powerups_;
        std::vector<Particle> particles_;

        // Game state
        GameState gameState_ = GameState::MENU;
        int collected_ = 0;
        int totalCheese_ = 0;
        int score_ = 0;
        int level_ = 1;
        float gameTime_ = 0.0f;
        float levelTime_ = 0.0f;
        float levelTimeLimit_ = 120.0f;
        float transitionTimer_ = 0.0f;

        // Power-ups
        bool mouseInvincible_ = false;
        bool mouseSpeedBoost_ = false;
        bool catFrozen_ = false;
        float powerUpTimer_ = 0.f;
        int currentPowerUp_ = -1;
        float powerUpSpawnTimer_ = 0.0f;

        // AI
        float aiUpdateTimer_ = 0.0f;
        glm::vec3 catTarget_;
        enum class CatState { PATROL, CHASE, CONFUSED } catState_ = CatState::PATROL;

        // Advanced systems
        std::unique_ptr<ParticleSystem> particleSystem_;
        std::unique_ptr<LightningSystem> lightningSystem_;
        std::unique_ptr<UIRenderer> uiRenderer_;
        float lightningCooldown_ = 0.0f;
    };

} // namespace game