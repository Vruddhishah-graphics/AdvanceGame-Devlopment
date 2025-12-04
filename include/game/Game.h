// Game.h - Enhanced with proper UI states and lightning effects
#pragma once
#include "game/pch.h"
#include "game/glm_minimal.h"
#include "game/Camera.h"
#include "game/MeshUtils.h"
#include "game/Texture.h"
#include "game/SoundSystem.h"
#include "game/ParticleSystem.h"
#include "game/LightningSystem.h"
#include "game/UIRenderer.h"
#include <memory>
#include <vector>
#include <string>

namespace game {

    enum class GameState {
        INTRO,           // "Press U to Play" screen
        PLAYING,
        PAUSED,
        LEVEL_TRANSITION,
        GAME_OVER,       // Game Over popup
        MOUSE_WIN,
        CAT_WIN
    };

    enum class CatState {
        PATROL,
        CHASE,
        CONFUSED
    };

    struct AABB { glm::vec3 min, max; };

    struct Entity {
        glm::vec3 pos{ 0 }, size{ 1 }, color{ 1 };
        float speed = 1.f;
        bool dynamic = true;
        float yaw = 0.0f;
        float pitch = 0.0f;
        int lives = 3;
        float invulnerabilityTimer = 0.0f;

        AABB bounds() const {
            glm::vec3 hs = size * 0.5f;
            return { pos - hs, pos + hs };
        }
    };

    struct Furniture {
        glm::vec3 pos{ 0 }, size{ 1 }, color{ 1 };
        bool dynamic = false;
        int type = 0;

        AABB bounds() const {
            glm::vec3 hs = size * 0.5f;
            return { pos - hs, pos + hs };
        }
    };

    struct Cheese {
        glm::vec3 pos{ 0 };
        bool taken = false;
        float rotation = 0.f;
        float bobOffset = 0.f;
    };

    struct PowerUp {
        glm::vec3 pos{ 0 };
        int type = 0;
        bool taken = false;
        float rotation = 0.f;
        float bobOffset = 0.f;
        float lifetime = 15.0f;
    };

    struct Particle {
        glm::vec3 pos{ 0 }, vel{ 0 }, color{ 1 };
        float life = 0.f, size = 0.1f;
    };

    class Game {
    public:
        void Run();

    private:
        // Window & GL
        GLFWwindow* win_ = nullptr;
        int width_ = 1280, height_ = 720;

        // Rendering
        GLuint prog_ = 0;
        GLint uModel_ = -1, uView_ = -1, uProj_ = -1, uViewPos_ = -1;
        GLint uBaseColor_ = -1, uEmissive_ = -1;
        GLint uKa_ = -1, uKd_ = -1, uKs_ = -1, uShine_ = -1;
        GLint uL1pos_ = -1, uL1col_ = -1, uL2pos_ = -1, uL2col_ = -1;
        GLint uUseTexture_ = -1, uTexture_ = -1;

        Camera cam_;
        float cameraAngle_ = 0.0f;
        float cameraHeight_ = 20.0f;
        float cameraDistance_ = 22.0f;

        // Meshes
        Mesh box_, sphere_, cyl_, cone_;
        Mesh mouseModel_, catModel_, cheeseModel_;

        // Textures
        std::unique_ptr<Texture> grassTex_, stoneTex_, metalTex_, woodTex_;

        // Advanced Systems
        std::unique_ptr<SoundSystem> soundSystem_;
        std::unique_ptr<ParticleSystem> particleSystem_;
        std::unique_ptr<LightningSystem> lightningSystem_;
        std::unique_ptr<UIRenderer> uiRenderer_;

        // Game State
        GameState gameState_ = GameState::INTRO;
        CatState catState_ = CatState::PATROL;

        // Game Objects
        Entity mouse_, cat_;
        glm::vec3 catTarget_{ 0 };
        std::vector<Entity> walls_;
        std::vector<Furniture> furniture_;
        std::vector<Cheese> cheeses_;
        std::vector<PowerUp> powerups_;
        std::vector<Particle> particles_;

        // Game Variables
        int level_ = 1;
        int score_ = 0;
        int collected_ = 0;
        int totalCheese_ = 5;
        float gameTime_ = 0.0f;
        float levelTime_ = 0.0f;
        float levelTimeLimit_ = 120.0f;
        float aiUpdateTimer_ = 0.0f;

        // Power-ups
        bool mouseInvincible_ = false;
        bool mouseSpeedBoost_ = false;
        bool catFrozen_ = false;
        float powerUpTimer_ = 0.0f;
        int currentPowerUp_ = -1;
        float powerUpSpawnTimer_ = 0.0f;

        // Visual Effects
        bool showCollisionEffect_ = false;
        float collisionEffectTimer_ = 0.0f;
        glm::vec3 collisionPosition_{ 0 };
        float introTimer_ = 10.0f;
        float transitionTimer_ = 0.0f;
        bool soundEnabled_ = true;

        // Game Over State
        float gameOverTimer_ = 0.0f;
        std::string gameOverMessage_ = "";
        bool showGameOverPopup_ = false;

        // Enhanced Lightning
        float lightningIntensity_ = 0.0f;
        float screenShakeAmount_ = 0.0f;

        // Input
        static bool keys_[1024];
        static void keyCb(GLFWwindow*, int key, int, int action, int);

        // Init
        void initWindow();
        void initGL();
        void initShaders();
        void initMeshes();
        void initTextures();
        void resetWorld();
        void startGame();
        void renderLevelTransition();
        void triggerVictoryCelebration();

        // Utility
        std::string loadText(const std::string& path);
        GLuint compile(GLenum type, const std::string& src);
        GLuint link(GLuint v, GLuint f);
        void printInstructions();

        // Update
        void loop();
        void update(float dt);
        void updateIntro(float dt);
        void updateMenu(float dt);
        void updatePlaying(float dt);
        void updatePaused(float dt);
        void updateGameOver(float dt);
        void updateCharacterRotations(float dt);
        void updateAI(float dt);
        void updatePhysics(float dt);
        void updatePowerUps(float dt);
        void nextLevel();
        void loseLife();
        void triggerCollisionEffect(const glm::vec3& pos);
        void triggerEnhancedLightning(const glm::vec3& pos);
        void checkCollisions();
        void checkWinConditions();

        // Physics
        bool intersects(const AABB& a, const AABB& b);
        glm::vec3 overlapVec(const AABB& a, const AABB& b);

        // Particles & Effects
        void spawnParticles(const glm::vec3& pos, const glm::vec3& color, int count);
        void spawnPowerUp();
        void applyPowerUp(int type);

        // Render
        void render();
        void renderScene();
        void renderIntro();
        void renderMenu();
        void renderPauseMenu();
        void renderGameOver();
        void renderGameOverPopup();
        void renderUI();
    };

} // namespace game