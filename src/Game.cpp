    // Game.cpp - PART 1 OF 4: Initialization and Setup
    #include "game/Game.h"
    #include <glm/gtc/matrix_transform.hpp>
    #include <glm/gtc/type_ptr.hpp>
    #include <cstdio>
    #include <cstdlib>
    #include <fstream>
    #include <sstream>
    #include <algorithm>
    #include <ctime>
    #include <iostream>
    #include <iomanip>
    #include <cmath>
    #include "game/pch.h"
    #include "game/glm_minimal.h"

    using namespace game;

    // Static members
    bool Game::keys_[1024] = { false };

    void Game::keyCb(GLFWwindow*, int key, int, int action, int) {
        if (key >= 0 && key < 1024) {
            if (action == GLFW_PRESS) keys_[key] = true;
            if (action == GLFW_RELEASE) keys_[key] = false;
        }
    }

    // ============================================================================
    // UTILITY FUNCTIONS
    // ============================================================================

    std::string Game::loadText(const std::string& path) {
        std::ifstream f(path);
        if (!f) {
            std::fprintf(stderr, "Failed to open %s\n", path.c_str());
            std::exit(1);
        }
        std::stringstream ss;
        ss << f.rdbuf();
        return ss.str();
    }

    GLuint Game::compile(GLenum type, const std::string& src) {
        GLuint s = glCreateShader(type);
        const char* c = src.c_str();
        glShaderSource(s, 1, &c, nullptr);
        glCompileShader(s);

        GLint ok = 0;
        glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            GLint n = 0;
            glGetShaderiv(s, GL_INFO_LOG_LENGTH, &n);
            std::string log(n, '\0');
            glGetShaderInfoLog(s, n, nullptr, log.data());
            std::fprintf(stderr, "Shader compile error:\n%s\n", log.c_str());
            std::exit(1);
        }
        return s;
    }

    GLuint Game::link(GLuint v, GLuint f) {
        GLuint p = glCreateProgram();
        glAttachShader(p, v);
        glAttachShader(p, f);
        glLinkProgram(p);

        GLint ok = 0;
        glGetProgramiv(p, GL_LINK_STATUS, &ok);
        if (!ok) {
            GLint n = 0;
            glGetProgramiv(p, GL_INFO_LOG_LENGTH, &n);
            std::string log(n, '\0');
            glGetProgramInfoLog(p, n, nullptr, log.data());
            std::fprintf(stderr, "Program link error:\n%s\n", log.c_str());
            std::exit(1);
        }

        glDeleteShader(v);
        glDeleteShader(f);
        return p;
    }

    void Game::printInstructions() {
        std::cout << "\n";
        std::cout << "================================================================\n";
        std::cout << "    TOM & JERRY: CHEESE CHASE 3D - ENHANCED EDITION            \n";
        std::cout << "================================================================\n";
        std::cout << " PRESS 'U' TO START THE GAME!                                  \n";
        std::cout << "================================================================\n";
        std::cout << " OBJECTIVE:                                                    \n";
        std::cout << "   Control Jerry (Mouse) and collect ALL cheese!               \n";
        std::cout << "   Avoid Tom (Cat) who hunts with balanced AI!                 \n";
        std::cout << "                                                               \n";
        std::cout << " CONTROLS:                                                     \n";
        std::cout << "   START:         U - Start Game                              \n";
        std::cout << "   Jerry:         W/A/S/D OR ARROW KEYS - Move                \n";
        std::cout << "   Camera:        Q/E - Rotate | Z/X - Height                 \n";
        std::cout << "   Game:          P - Pause | R - Restart | ESC - Quit        \n";
        std::cout << "   Sound:         M - Toggle Sound ON/OFF                     \n";
        std::cout << "                                                               \n";
        std::cout << " POWER-UPS (Last 5 seconds):                                  \n";
        std::cout << "   Gold Sphere  - SHIELD (Invincible)                         \n";
        std::cout << "   Cyan Cone    - SPEED BOOST (1.5x speed)                    \n";
        std::cout << "   Blue Sphere  - FREEZE TOM (3 seconds)                      \n";
        std::cout << "================================================================\n";
        std::cout << "\n";
    }

    // ============================================================================
    // MAIN RUN FUNCTION
    // ============================================================================

    void Game::Run() {
        std::srand((unsigned int)std::time(nullptr));

        initWindow();
        initGL();
        initShaders();
        initMeshes();
        initTextures();

        std::cout << "Initializing advanced systems...\n";

        try {
            soundSystem_ = std::make_unique<SoundSystem>();
            soundSystem_->Init();
            std::cout << "  Sound system initialized\n";
        }
        catch (const std::exception& e) {
            std::cerr << "  Sound system failed: " << e.what() << "\n";
        }

        try {
            particleSystem_ = std::make_unique<ParticleSystem>(2000);
            particleSystem_->Init();
            std::cout << "  Particle system initialized\n";
        }
        catch (const std::exception& e) {
            std::cerr << "  Particle system failed: " << e.what() << "\n";
        }

        try {
            lightningSystem_ = std::make_unique<LightningSystem>();
            lightningSystem_->Init();
            std::cout << "  Lightning system initialized\n";
        }
        catch (const std::exception& e) {
            std::cerr << "  Lightning system failed: " << e.what() << "\n";
        }

        try {
            uiRenderer_ = std::make_unique<UIRenderer>();
            uiRenderer_->Init(width_, height_);
            std::cout << "  UI Renderer initialized\n";
        }
        catch (const std::exception& e) {
            std::cerr << "  UI Renderer failed: " << e.what() << "\n";
        }

        printInstructions();
        resetWorld();

        gameState_ = GameState::INTRO;

        loop();

        glfwDestroyWindow(win_);
        glfwTerminate();
    }

    // ============================================================================
    // INITIALIZATION FUNCTIONS
    // ============================================================================

    void Game::initWindow() {
        if (!glfwInit()) {
            std::fprintf(stderr, "GLFW init failed\n");
            std::exit(1);
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    #if defined(__APPLE__)
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

        win_ = glfwCreateWindow(width_, height_, "Tom & Jerry: Cheese Chase 3D - Enhanced", nullptr, nullptr);
        if (!win_) {
            std::fprintf(stderr, "Failed to create window\n");
            std::exit(1);
        }

        glfwMakeContextCurrent(win_);
        glfwSwapInterval(1);
        glfwSetKeyCallback(win_, keyCb);
    }

    void Game::initGL() {
        glewExperimental = GL_TRUE;
        GLenum ge = glewInit();
        glGetError();
        if (ge != GLEW_OK) {
            std::fprintf(stderr, "GLEW init failed: %s\n", (const char*)glewGetErrorString(ge));
            std::exit(1);
        }

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void Game::initShaders() {
        std::string base = ASSET_DIR;
        std::string vsPath = base + std::string("/shaders/basic.vert");
        std::string fsPath = base + std::string("/shaders/basic.frag");

        std::string vsSrc = loadText(vsPath);
        std::string fsSrc = loadText(fsPath);

        GLuint v = compile(GL_VERTEX_SHADER, vsSrc);
        GLuint f = compile(GL_FRAGMENT_SHADER, fsSrc);
        prog_ = link(v, f);

        glUseProgram(prog_);

        uModel_ = glGetUniformLocation(prog_, "uModel");
        uView_ = glGetUniformLocation(prog_, "uView");
        uProj_ = glGetUniformLocation(prog_, "uProj");
        uViewPos_ = glGetUniformLocation(prog_, "uViewPos");
        uBaseColor_ = glGetUniformLocation(prog_, "uBaseColor");
        uEmissive_ = glGetUniformLocation(prog_, "uEmissive");
        uKa_ = glGetUniformLocation(prog_, "uKa");
        uKd_ = glGetUniformLocation(prog_, "uKd");
        uKs_ = glGetUniformLocation(prog_, "uKs");
        uShine_ = glGetUniformLocation(prog_, "uShine");
        uL1pos_ = glGetUniformLocation(prog_, "uL1.position");
        uL1col_ = glGetUniformLocation(prog_, "uL1.color");
        uL2pos_ = glGetUniformLocation(prog_, "uL2.position");
        uL2col_ = glGetUniformLocation(prog_, "uL2.color");
        uUseTexture_ = glGetUniformLocation(prog_, "uUseTexture");
        uTexture_ = glGetUniformLocation(prog_, "uTexture");

        glm::vec3 L1pos(6.0f, 12.0f, 6.0f);
        glm::vec3 L1col(6.0f, 5.5f, 5.0f);
        glm::vec3 L2pos(-6.0f, 10.0f, -6.0f);
        glm::vec3 L2col(4.0f, 4.5f, 5.0f);

        glUniform3fv(uL1pos_, 1, glm::value_ptr(L1pos));
        glUniform3fv(uL1col_, 1, glm::value_ptr(L1col));
        glUniform3fv(uL2pos_, 1, glm::value_ptr(L2pos));
        glUniform3fv(uL2col_, 1, glm::value_ptr(L2col));
    }

    void Game::initMeshes() {
        box_ = makeBox();
        sphere_ = makeSphere(24, 16);
        cyl_ = makeCylinder(24);
        cone_ = makeCone(24);

        std::string base = ASSET_DIR;
        std::cout << "Loading 3D models...\n";
        mouseModel_ = loadOBJ(base + std::string("/models/mouse.obj"));
        catModel_ = loadOBJ(base + std::string("/models/cat.obj"));
        cheeseModel_ = loadOBJ(base + std::string("/models/cheese.obj"));
        std::cout << "Models ready!\n";
    }

    void Game::initTextures() {
        std::cout << "Generating procedural textures...\n";
        grassTex_ = std::make_unique<Texture>();
        grassTex_->GenerateGrass();
        stoneTex_ = std::make_unique<Texture>();
        stoneTex_->GenerateStone();
        metalTex_ = std::make_unique<Texture>();
        metalTex_->GenerateMetal();
        woodTex_ = std::make_unique<Texture>();
        woodTex_->GenerateCheckerboard();
        std::cout << "Textures generated!\n";
    }

    void Game::resetWorld() {
        cam_.setProjection(45.f, float(width_) / float(height_), 0.1f, 100.f);

        cameraAngle_ = 3.14159f / 2.0f;
        cameraHeight_ = 20.0f;
        cameraDistance_ = 22.0f;

        glm::vec3 camPos(
            cameraDistance_ * std::cos(cameraAngle_),
            cameraHeight_,
            cameraDistance_ * std::sin(cameraAngle_)
        );
        cam_.setPosition(camPos);
        cam_.setTarget(glm::vec3(0.f, 0.f, 0.f));

        mouse_ = {};
        mouse_.pos = { -4.f, 0.4f, -2.f };
        mouse_.size = { 0.9f, 0.9f, 0.9f };
        mouse_.color = { 0.92f, 0.92f, 1.0f };
        mouse_.speed = 5.5f;
        mouse_.lives = 3;
        mouse_.yaw = 0.0f;
        mouse_.pitch = 0.0f;

        cat_ = {};
        cat_.pos = { 3.5f, 0.4f, 2.0f };
        cat_.size = { 1.0f, 1.2f, 1.0f };
        cat_.color = { 1.0f, 0.63f, 0.35f };
        cat_.speed = 4.0f + level_ * 0.2f;
        cat_.yaw = 0.0f;
        cat_.pitch = 0.0f;
        catState_ = CatState::PATROL;
        catTarget_ = cat_.pos;

        walls_.clear();
        auto wall = [&](float x, float z, float sx, float sz) {
            Entity w;
            w.pos = { x, 0.75f, z };
            w.size = { sx, 1.5f, sz };
            w.color = { 1.0f, 0.96f, 0.75f };
            w.dynamic = false;
            walls_.push_back(w);
            };
        const float W = 18.f;
        const float D = 12.f;
        wall(0.f, -D * 0.5f, W, 0.8f);
        wall(0.f, D * 0.5f, W, 0.8f);
        wall(-W * 0.5f, 0.f, 0.8f, D);
        wall(W * 0.5f, 0.f, 0.8f, D);

        furniture_.clear();
        auto addF = [&](const glm::vec3& p, const glm::vec3& s, const glm::vec3& c, int type) {
            Furniture f;
            f.pos = p;
            f.size = s;
            f.color = c;
            f.dynamic = false;
            f.type = type;
            furniture_.push_back(f);
            };
        addF({ -4.0f, 0.5f, -1.5f }, { 2.0f, 1.0f, 1.2f }, { 0.72f, 0.52f, 0.36f }, 0);
        addF({ 0.0f, 0.6f, 0.0f }, { 3.0f, 1.2f, 1.0f }, { 0.86f, 0.57f, 0.40f }, 0);
        addF({ 2.0f, 0.5f, 2.5f }, { 1.7f, 1.0f, 1.5f }, { 0.45f, 0.64f, 0.86f }, 1);
        addF({ -2.5f, 0.5f, 3.0f }, { 1.5f, 1.0f, 1.0f }, { 0.65f, 0.45f, 0.35f }, 2);

        cheeses_.clear();
        totalCheese_ = 5 + level_;
        for (int i = 0; i < totalCheese_; ++i) {
            float x = -7.f + (rand() % 140) / 10.0f;
            float z = -5.f + (rand() % 100) / 10.0f;
            Cheese c;
            c.pos = glm::vec3(x, 0.35f, z);
            c.taken = false;
            c.rotation = (float)(rand() % 360);
            cheeses_.push_back(c);
        }

        powerups_.clear();
        for (int i = 0; i < 2; ++i) {
            PowerUp p;
            p.pos = glm::vec3(
                -7.f + (rand() % 140) / 10.0f,
                0.6f,
                -5.f + (rand() % 100) / 10.0f
            );
            p.type = rand() % 3;
            p.taken = false;
            p.rotation = 0.f;
            powerups_.push_back(p);
        }

        particles_.clear();
        collected_ = 0;
        levelTime_ = 0.0f;

        mouseInvincible_ = false;
        mouseSpeedBoost_ = false;
        catFrozen_ = false;
        powerUpTimer_ = 0.f;
        currentPowerUp_ = -1;

        showCollisionEffect_ = false;
        collisionEffectTimer_ = 0.0f;

        if (level_ > 1) {
            std::cout << "\n>>> LEVEL " << level_ << " STARTED! <<<\n\n";
        }
    }

    void Game::startGame() {
        gameState_ = GameState::PLAYING;
        level_ = 1;
        score_ = 0;
        gameTime_ = 0.0f;
        showGameOverPopup_ = false;
        gameOverTimer_ = 0.0f;

        resetWorld();

        // START THE TOM & JERRY THEME!
        if (soundSystem_ && soundEnabled_) {
            std::cout << "Starting Tom & Jerry theme music...\n";
            soundSystem_->StopMusic(); // Stop any existing music first
            soundSystem_->PlayBackgroundMusic();

            // DEBUG: Check if music is actually playing
            std::cout << "Music playback initiated\n";
        }
        else {
            std::cout << "Sound system not available or disabled\n";
            std::cout << "   soundSystem_: " << (soundSystem_ ? "OK" : "NULL") << "\n";
            std::cout << "   soundEnabled_: " << (soundEnabled_ ? "YES" : "NO") << "\n";
        }
    }
    void Game::nextLevel() {
        int timeBonus = (int)(levelTimeLimit_ - levelTime_) * 10;
        if (timeBonus > 0) score_ += timeBonus;
        score_ += 500;

        level_++;
        transitionTimer_ = 3.0f; // 3 seconds for transition
        gameState_ = GameState::LEVEL_TRANSITION;

        // Celebration effects!
        triggerVictoryCelebration();

        std::cout << "\n>>> LEVEL " << (level_ - 1) << " COMPLETE! <<<\n";
        std::cout << "Time Bonus: +" << timeBonus << "\n";
        std::cout << "Level Bonus: +500\n";
        std::cout << "Total Score: " << score_ << "\n\n";

        if (soundSystem_ && soundEnabled_) {
            soundSystem_->Play(SoundSystem::LEVEL_COMPLETE);
        }
    }


    // Replace loseLife() function in Game.cpp:

    void Game::loseLife() {
        mouse_.lives--;
        mouse_.invulnerabilityTimer = 2.0f;

        triggerEnhancedLightning(mouse_.pos);

        if (soundSystem_ && soundEnabled_) {
            soundSystem_->Play(SoundSystem::LOSE_LIFE);
        }

        if (mouse_.lives <= 0) {
            // Set game over state
            gameState_ = GameState::GAME_OVER;
            showGameOverPopup_ = true;
            gameOverMessage_ = "TOM WINS!";
            gameOverTimer_ = 0.0f;

            std::cout << "\n>>> GAME OVER - TOM CAUGHT JERRY! <<<\n";

            if (soundSystem_ && soundEnabled_) {
                soundSystem_->Play(SoundSystem::GAME_LOSE);
            }
        }
        else {
            // Still has lives, respawn
            mouse_.pos = { -4.f, 0.4f, -2.f };
            std::cout << "Lives remaining: " << mouse_.lives << "\n";
        }
    }

    void Game::triggerCollisionEffect(const glm::vec3& pos) {
        showCollisionEffect_ = true;
        collisionEffectTimer_ = 0.5f;
        collisionPosition_ = pos;

        spawnParticles(pos, glm::vec3(1.0f, 0.3f, 0.0f), 150);
        spawnParticles(pos, glm::vec3(1.0f, 1.0f, 0.0f), 100);

        if (lightningSystem_) {
            for (int i = 0; i < 5; ++i) {
                glm::vec3 offset(
                    (rand() % 100 - 50) / 50.0f,
                    (rand() % 100) / 50.0f,
                    (rand() % 100 - 50) / 50.0f
                );
                lightningSystem_->TriggerLightning(pos + glm::vec3(0, 2, 0), pos + offset);
            }
        }
    }

    void Game::triggerEnhancedLightning(const glm::vec3& pos) {
        lightningIntensity_ = 1.0f;
        screenShakeAmount_ = 0.3f;

        if (lightningSystem_) {
            for (int i = 0; i < 8; ++i) {
                glm::vec3 offset(
                    (rand() % 200 - 100) / 30.0f,
                    (rand() % 150) / 30.0f,
                    (rand() % 200 - 100) / 30.0f
                );

                glm::vec3 start = pos + glm::vec3(0, 3, 0) + offset * 0.3f;
                glm::vec3 end = pos + offset;

                lightningSystem_->TriggerLightning(start, end);
            }
        }

        spawnParticles(pos, glm::vec3(1.0f, 1.0f, 0.3f), 200);
        spawnParticles(pos, glm::vec3(0.3f, 0.5f, 1.0f), 150);
        spawnParticles(pos, glm::vec3(1.0f, 0.3f, 0.3f), 100);
    }

    void Game::updateCharacterRotations(float dt) {
        if (keys_[GLFW_KEY_W] || keys_[GLFW_KEY_UP]) {
            mouse_.yaw = 0.0f;
        }
        if (keys_[GLFW_KEY_S] || keys_[GLFW_KEY_DOWN]) {
            mouse_.yaw = 180.0f;
        }
        if (keys_[GLFW_KEY_A] || keys_[GLFW_KEY_LEFT]) {
            mouse_.yaw = -90.0f;
        }
        if (keys_[GLFW_KEY_D] || keys_[GLFW_KEY_RIGHT]) {
            mouse_.yaw = 90.0f;
        }

        glm::vec3 dirToCatTarget = catTarget_ - cat_.pos;
        if (glm::length(dirToCatTarget) > 0.1f) {
            cat_.yaw = std::atan2(dirToCatTarget.x, dirToCatTarget.z) * 180.0f / 3.14159f;
        }

        cat_.pitch = std::sin(gameTime_ * 2.0f) * 5.0f;
        mouse_.pitch = std::sin(gameTime_ * 3.0f) * 3.0f;
    }

    void Game::updateAI(float dt) {
        if (catFrozen_) {
            catState_ = CatState::CONFUSED;
            return;
        }

        aiUpdateTimer_ += dt;
        float distToMouse = glm::length(mouse_.pos - cat_.pos);

        if (aiUpdateTimer_ >= 0.3f) {
            aiUpdateTimer_ = 0.0f;

            switch (catState_) {
            case CatState::PATROL: {
                if (glm::length(catTarget_ - cat_.pos) < 0.5f || rand() % 100 < 10) {
                    catTarget_ = glm::vec3(
                        -7.f + (rand() % 140) / 10.0f,
                        0.4f,
                        -5.f + (rand() % 100) / 10.0f
                    );
                }

                if (distToMouse < 10.0f) {
                    catState_ = CatState::CHASE;
                    if (soundSystem_ && soundEnabled_) {
                        soundSystem_->Play(SoundSystem::CAT_CHASE);
                    }
                }
                break;
            }

            case CatState::CHASE: {
                catTarget_ = mouse_.pos;
                if (distToMouse > 15.0f) {
                    catState_ = CatState::PATROL;
                }
                break;
            }

            case CatState::CONFUSED: {
                if (glm::length(catTarget_ - cat_.pos) < 0.5f) {
                    catTarget_ = glm::vec3(
                        cat_.pos.x + ((rand() % 100 - 50) / 10.0f),
                        0.4f,
                        cat_.pos.z + ((rand() % 100 - 50) / 10.0f)
                    );
                }

                if (!catFrozen_ && rand() % 100 < 5) {
                    catState_ = CatState::PATROL;
                }
                break;
            }
            }
        }

        glm::vec3 direction = catTarget_ - cat_.pos;
        direction.y = 0;

        if (glm::length(direction) > 0.1f) {
            direction = glm::normalize(direction);
            cat_.pos += direction * cat_.speed * dt;
        }
    }

    void Game::spawnParticles(const glm::vec3& pos, const glm::vec3& color, int count) {
        if (particleSystem_) {
            glm::vec4 particleColor(color.r, color.g, color.b, 1.0f);
            particleSystem_->CreateExplosion(pos, particleColor, count);
        }
        else {
            for (int i = 0; i < count; ++i) {
                Particle p;
                p.pos = pos;
                float angle = (float)(std::rand() % 360) * 3.14159265f / 180.f;
                float speed = 2.f + (float)(std::rand() % 100) / 50.f;
                p.vel = glm::vec3(
                    std::cos(angle) * speed,
                    3.f + (float)(std::rand() % 100) / 50.f,
                    std::sin(angle) * speed
                );
                p.color = color;
                p.life = 1.0f;
                p.size = 0.1f + (float)(std::rand() % 100) / 500.f;
                particles_.push_back(p);
            }
        }
    }

    void Game::spawnPowerUp() {
        if (powerups_.size() >= 3) return;

        PowerUp p;
        p.pos = glm::vec3(
            -7.f + (rand() % 140) / 10.0f,
            0.6f,
            -5.f + (rand() % 100) / 10.0f
        );
        p.type = rand() % 3;
        p.taken = false;
        p.rotation = 0.f;
        p.lifetime = 15.0f;
        powerups_.push_back(p);
    }

    void Game::applyPowerUp(int type) {
        currentPowerUp_ = type;
        powerUpTimer_ = 5.0f;

        switch (type) {
        case 0:
            mouseInvincible_ = true;
            std::cout << "✨ SHIELD ACTIVATED!\n";
            if (soundSystem_ && soundEnabled_) {
                soundSystem_->Play(SoundSystem::SHIELD_ACTIVE);
            }
            break;
        case 1:
            mouseSpeedBoost_ = true;
            std::cout << "⚡ SPEED BOOST!\n";
            if (soundSystem_ && soundEnabled_) {
                soundSystem_->Play(SoundSystem::SPEED_BOOST);
            }
            break;
        case 2:
            catFrozen_ = true;
            catState_ = CatState::CONFUSED;
            powerUpTimer_ = 3.0f;
            std::cout << "❄️ TOM FROZEN!\n";
            if (soundSystem_ && soundEnabled_) {
                soundSystem_->Play(SoundSystem::FREEZE_EFFECT);
            }
            break;
        }
    }

    void Game::updatePowerUps(float dt) {
        if (powerUpTimer_ > 0.f) {
            powerUpTimer_ -= dt;
            if (powerUpTimer_ <= 0.f) {
                mouseInvincible_ = false;
                mouseSpeedBoost_ = false;
                catFrozen_ = false;
                currentPowerUp_ = -1;
            }
        }

        powerUpSpawnTimer_ += dt;
        if (powerUpSpawnTimer_ > 10.0f) {
            powerUpSpawnTimer_ = 0.0f;
            spawnPowerUp();
        }

        for (auto it = powerups_.begin(); it != powerups_.end();) {
            if (!it->taken) {
                it->rotation += dt * 2.0f;
                it->bobOffset = std::sin(gameTime_ * 3.0f + it->rotation) * 0.1f;
                it->lifetime -= dt;

                if (it->lifetime <= 0.0f) {
                    it = powerups_.erase(it);
                    continue;
                }
            }
            ++it;
        }
    }

    void Game::checkCollisions() {
        for (auto& c : cheeses_) {
            if (c.taken) continue;
            AABB mouseBox = mouse_.bounds();
            AABB cheeseBox{ c.pos - glm::vec3(0.3f), c.pos + glm::vec3(0.3f) };
            if (intersects(mouseBox, cheeseBox)) {
                c.taken = true;
                collected_++;
                score_ += 100;
                spawnParticles(c.pos, glm::vec3(1.0f, 0.95f, 0.2f), 20);

                if (soundSystem_ && soundEnabled_) {
                    soundSystem_->Play(SoundSystem::CHEESE_COLLECT);
                }
            }
        }

        for (auto& p : powerups_) {
            if (p.taken) continue;
            AABB mouseBox = mouse_.bounds();
            AABB powerUpBox{ p.pos - glm::vec3(0.3f), p.pos + glm::vec3(0.3f) };
            if (intersects(mouseBox, powerUpBox)) {
                p.taken = true;
                applyPowerUp(p.type);
                spawnParticles(p.pos, glm::vec3(1.0f, 0.84f, 0.0f), 30);

                if (soundSystem_ && soundEnabled_) {
                    soundSystem_->Play(SoundSystem::POWERUP_PICKUP);
                }
            }
        }

        if (mouse_.invulnerabilityTimer <= 0.0f) {
            if (glm::length(mouse_.pos - cat_.pos) < 1.0f) {
                if (!mouseInvincible_) {
                    loseLife();
                }
            }
        }
    }

    // Replace checkWinConditions() function in Game.cpp:

    void Game::checkWinConditions() {
        // Check if all cheese collected
        if (collected_ >= totalCheese_) {
            if (level_ >= 3) {
                // Won the game!
                gameState_ = GameState::MOUSE_WIN;
                showGameOverPopup_ = true;
                gameOverMessage_ = "JERRY WINS!";
                gameOverTimer_ = 0.0f;

                // MEGA VICTORY CELEBRATION!
                triggerVictoryCelebration();

                std::cout << "\n>>> JERRY WINS THE GAME! <<<\n";

                if (soundSystem_ && soundEnabled_) {
                    soundSystem_->Play(SoundSystem::GAME_WIN);
                }
            }
            else {
                // Advance to next level
                nextLevel();
            }
        }

        // Check if time ran out
        if (levelTime_ >= levelTimeLimit_) {
            gameState_ = GameState::CAT_WIN;
            showGameOverPopup_ = true;
            gameOverMessage_ = "TIME'S UP! TOM WINS!";
            gameOverTimer_ = 0.0f;

            std::cout << "\n>>> TIME'S UP! TOM WINS! <<<\n";

            if (soundSystem_ && soundEnabled_) {
                soundSystem_->Play(SoundSystem::GAME_LOSE);
            }
        }
    }



    bool Game::intersects(const AABB& a, const AABB& b) {
        return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
            (a.min.y <= b.max.y && a.max.y >= b.min.y) &&
            (a.min.z <= b.max.z && a.max.z >= b.min.z);
    }

    glm::vec3 Game::overlapVec(const AABB& a, const AABB& b) {
        float overlapX = std::min(a.max.x - b.min.x, b.max.x - a.min.x);
        float overlapY = std::min(a.max.y - b.min.y, b.max.y - a.min.y);
        float overlapZ = std::min(a.max.z - b.min.z, b.max.z - a.min.z);

        if (overlapX <= 0 || overlapY <= 0 || overlapZ <= 0) {
            return glm::vec3(0);
        }

        if (overlapX < overlapY && overlapX < overlapZ) {
            return glm::vec3((a.min.x < b.min.x) ? -overlapX : overlapX, 0, 0);
        }
        else if (overlapY < overlapZ) {
            return glm::vec3(0, (a.min.y < b.min.y) ? -overlapY : overlapY, 0);
        }
        else {
            return glm::vec3(0, 0, (a.min.z < b.min.z) ? -overlapZ : overlapZ);
        }
    }

    void Game::updatePhysics(float dt) {
        const float roomMinX = -8.5f;
        const float roomMaxX = 8.5f;
        const float roomMinZ = -5.5f;
        const float roomMaxZ = 5.5f;

        for (const auto& wall : walls_) {
            AABB mouseBox = mouse_.bounds();
            AABB wallBox = wall.bounds();

            if (intersects(mouseBox, wallBox)) {
                glm::vec3 separation = overlapVec(mouseBox, wallBox);
                mouse_.pos += separation;
            }
        }

        for (const auto& wall : walls_) {
            AABB catBox = cat_.bounds();
            AABB wallBox = wall.bounds();

            if (intersects(catBox, wallBox)) {
                glm::vec3 separation = overlapVec(catBox, wallBox);
                cat_.pos += separation;
            }
        }

        for (const auto& furn : furniture_) {
            AABB mouseBox = mouse_.bounds();
            AABB furnBox = furn.bounds();

            if (intersects(mouseBox, furnBox)) {
                glm::vec3 separation = overlapVec(mouseBox, furnBox);
                mouse_.pos += separation;
            }
        }

        for (const auto& furn : furniture_) {
            AABB catBox = cat_.bounds();
            AABB furnBox = furn.bounds();

            if (intersects(catBox, furnBox)) {
                glm::vec3 separation = overlapVec(catBox, furnBox);
                cat_.pos += separation;
            }
        }

        float mouseHalfSizeX = mouse_.size.x * 0.5f;
        float mouseHalfSizeZ = mouse_.size.z * 0.5f;

        mouse_.pos.x = glm::clamp(mouse_.pos.x, roomMinX + mouseHalfSizeX, roomMaxX - mouseHalfSizeX);
        mouse_.pos.z = glm::clamp(mouse_.pos.z, roomMinZ + mouseHalfSizeZ, roomMaxZ - mouseHalfSizeZ);
        mouse_.pos.y = 0.4f;

        float catHalfSizeX = cat_.size.x * 0.5f;
        float catHalfSizeZ = cat_.size.z * 0.5f;

        cat_.pos.x = glm::clamp(cat_.pos.x, roomMinX + catHalfSizeX, roomMaxX - catHalfSizeX);
        cat_.pos.z = glm::clamp(cat_.pos.z, roomMinZ + catHalfSizeZ, roomMaxZ - catHalfSizeZ);
        cat_.pos.y = 0.4f;
    }
    // Game.cpp - PART 3 OF 4: Main Loop and Update Functions

    void Game::loop() {
        double prev = glfwGetTime();
        while (!glfwWindowShouldClose(win_)) {
            double now = glfwGetTime();
            float dt = static_cast<float>(now - prev);
            prev = now;

            update(dt);
            render();

            glfwSwapBuffers(win_);
            glfwPollEvents();
        }
    }

    void Game::update(float dt) {
        if (keys_[GLFW_KEY_ESCAPE])
            glfwSetWindowShouldClose(win_, GL_TRUE);

        if (keys_[GLFW_KEY_M]) {
            soundEnabled_ = !soundEnabled_;
            std::cout << "🔊 Sound: " << (soundEnabled_ ? "ON" : "OFF") << "\n";
            keys_[GLFW_KEY_M] = false;
        }

        if (keys_[GLFW_KEY_Q]) cameraAngle_ -= 1.5f * dt;
        if (keys_[GLFW_KEY_E]) cameraAngle_ += 1.5f * dt;

        float minHeight = 10.0f;
        float maxHeight = 40.0f;
        if (keys_[GLFW_KEY_Z]) {
            cameraHeight_ -= 8.0f * dt;
            if (cameraHeight_ < minHeight) cameraHeight_ = minHeight;
        }
        if (keys_[GLFW_KEY_X]) {
            cameraHeight_ += 8.0f * dt;
            if (cameraHeight_ > maxHeight) cameraHeight_ = maxHeight;
        }

        glm::vec3 shakeOffset(0);
        if (screenShakeAmount_ > 0.0f) {
            screenShakeAmount_ -= dt * 2.0f;
            shakeOffset = glm::vec3(
                (rand() % 100 - 50) / 500.0f * screenShakeAmount_,
                (rand() % 100 - 50) / 500.0f * screenShakeAmount_,
                (rand() % 100 - 50) / 500.0f * screenShakeAmount_
            );
        }

        glm::vec3 camPos(
            cameraDistance_ * std::cos(cameraAngle_),
            cameraHeight_,
            cameraDistance_ * std::sin(cameraAngle_)
        );
        cam_.setPosition(camPos + shakeOffset);
        cam_.setTarget(glm::vec3(0.f, 0.f, 0.f));

        if (lightningIntensity_ > 0.0f) {
            lightningIntensity_ -= dt * 2.0f;
        }

        if (particleSystem_) particleSystem_->Update(dt);
        if (lightningSystem_) lightningSystem_->Update(dt);

        if (showCollisionEffect_) {
            collisionEffectTimer_ -= dt;
            if (collisionEffectTimer_ <= 0.0f) {
                showCollisionEffect_ = false;
            }
        }

        switch (gameState_) {
        case GameState::INTRO:
            updateIntro(dt);
            break;
        case GameState::PLAYING:
            updatePlaying(dt);
            break;
        case GameState::PAUSED:
            updatePaused(dt);
            break;
        case GameState::LEVEL_TRANSITION:
            transitionTimer_ -= dt;
            if (transitionTimer_ <= 0.0f) {
                resetWorld();
                gameState_ = GameState::PLAYING;
            }
            break;
        case GameState::MOUSE_WIN:
        case GameState::CAT_WIN:
        case GameState::GAME_OVER:
            updateGameOver(dt);
            break;
        }

        std::string title = "Tom & Jerry 3D | Level:" + std::to_string(level_) +
            " | Score:" + std::to_string(score_) +
            " | Cheese:" + std::to_string(collected_) + "/" + std::to_string(totalCheese_) +
            " | Lives:" + std::to_string(mouse_.lives);

        if (currentPowerUp_ >= 0) {
            title += " | PowerUp:";
            if (currentPowerUp_ == 0) title += "SHIELD";
            else if (currentPowerUp_ == 1) title += "SPEED";
            else title += "FREEZE";
            title += "(" + std::to_string((int)std::ceil(powerUpTimer_)) + "s)";
        }

        glfwSetWindowTitle(win_, title.c_str());
    }

    void Game::updateIntro(float dt) {
        introTimer_ -= dt;

        if (keys_[GLFW_KEY_U]) {
            startGame();
            keys_[GLFW_KEY_U] = false;
        }

        if (introTimer_ <= 0.0f) {
            introTimer_ = 10.0f;
        }
    }

    void Game::updatePlaying(float dt) {
        gameTime_ += dt;
        levelTime_ += dt;

        if (keys_[GLFW_KEY_P]) {
            gameState_ = GameState::PAUSED;
            keys_[GLFW_KEY_P] = false;
            return;
        }

        if (mouse_.invulnerabilityTimer > 0.0f) {
            mouse_.invulnerabilityTimer -= dt;
        }

        glm::vec3 mouseMove(0);

        if (keys_[GLFW_KEY_W] || keys_[GLFW_KEY_UP]) mouseMove.z -= 1;
        if (keys_[GLFW_KEY_S] || keys_[GLFW_KEY_DOWN]) mouseMove.z += 1;
        if (keys_[GLFW_KEY_A] || keys_[GLFW_KEY_LEFT]) mouseMove.x -= 1;
        if (keys_[GLFW_KEY_D] || keys_[GLFW_KEY_RIGHT]) mouseMove.x += 1;

        if (glm::length(mouseMove) > 0.f) {
            mouseMove = glm::normalize(mouseMove);

            float currentSpeed = mouse_.speed;
            if (mouseSpeedBoost_) currentSpeed *= 1.5f;

            mouse_.pos += mouseMove * currentSpeed * dt;
        }

        updateCharacterRotations(dt);
        updateAI(dt);
        updatePhysics(dt);
        updatePowerUps(dt);

        for (auto& c : cheeses_) {
            if (!c.taken) {
                c.rotation += dt * 1.5f;
                c.bobOffset = std::sin(gameTime_ * 2.0f + c.rotation) * 0.08f;
            }
        }

        for (auto it = particles_.begin(); it != particles_.end();) {
            it->pos += it->vel * dt;
            it->vel.y -= 9.8f * dt;
            it->life -= dt;
            if (it->life <= 0.f) it = particles_.erase(it);
            else ++it;
        }

        checkCollisions();
        checkWinConditions();
    }


    void Game::updatePaused(float dt) {
        // Pause the music when game is paused
        static bool wasPaused = false;

        if (!wasPaused) {
            if (soundSystem_) {
                soundSystem_->PauseMusic();
            }
            wasPaused = true;
        }

        if (keys_[GLFW_KEY_P]) {
            gameState_ = GameState::PLAYING;
            keys_[GLFW_KEY_P] = false;

            // Resume music
            if (soundSystem_) {
                soundSystem_->ResumeMusic();
            }
            wasPaused = false;
        }
    }
    void Game::updateGameOver(float dt) {
        gameOverTimer_ += dt;

        // Stop music when game is over
        static bool musicStopped = false;
        if (!musicStopped && soundSystem_) {
            soundSystem_->StopMusic();
            musicStopped = true;
        }

        // Victory celebration effects
        if (gameState_ == GameState::MOUSE_WIN && gameOverTimer_ < 3.0f) {
            if (particleSystem_ && (int)(gameOverTimer_ * 10) % 2 == 0) {
                for (int i = 0; i < 2; ++i) {
                    glm::vec3 rainPos(
                        (rand() % 160 - 80) / 10.0f,
                        8.0f,
                        (rand() % 120 - 60) / 10.0f
                    );
                    glm::vec4 color(
                        (rand() % 100) / 100.0f + 0.3f,
                        (rand() % 100) / 100.0f + 0.3f,
                        (rand() % 100) / 100.0f + 0.3f,
                        1.0f
                    );
                    particleSystem_->CreateExplosion(rainPos, color, 5);
                }
            }
        }

        if (keys_[GLFW_KEY_R]) {
            musicStopped = false;  // Reset flag
            startGame();
            keys_[GLFW_KEY_R] = false;
        }
    }


    // Replace the render() function in Game.cpp with this fixed version:


    void Game::render() {
        int W, H;
        glfwGetFramebufferSize(win_, &W, &H);
        glViewport(0, 0, W, H);

        float clearR = 0.52f;
        float clearG = 0.76f;
        float clearB = 0.92f;

        if (gameState_ == GameState::PLAYING || gameState_ == GameState::PAUSED) {
            clearR += lightningIntensity_ * 0.4f;
            clearG += lightningIntensity_ * 0.2f;
            clearB += lightningIntensity_ * 0.08f;
        }
        else if (gameState_ == GameState::INTRO) {
            clearR = 0.15f;
            clearG = 0.15f;
            clearB = 0.25f;
        }
        else if (gameState_ == GameState::GAME_OVER ||
            gameState_ == GameState::MOUSE_WIN ||
            gameState_ == GameState::CAT_WIN) {
            clearR = 0.05f;
            clearG = 0.05f;
            clearB = 0.1f;
        }
        else if (gameState_ == GameState::LEVEL_TRANSITION) {
            // Bright celebratory background
            clearR = 0.7f + lightningIntensity_ * 0.3f;
            clearG = 0.8f + lightningIntensity_ * 0.2f;
            clearB = 0.9f + lightningIntensity_ * 0.1f;
        }

        glClearColor(clearR, clearG, clearB, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (gameState_ == GameState::INTRO) {
            renderIntro();
            return;
        }

        // Render 3D scene for all game states
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glDisable(GL_BLEND);

        glUseProgram(prog_);
        glm::mat4 V = cam_.view();
        glm::mat4 P = cam_.proj();
        glUniformMatrix4fv(uView_, 1, GL_FALSE, glm::value_ptr(V));
        glUniformMatrix4fv(uProj_, 1, GL_FALSE, glm::value_ptr(P));
        glUniform3fv(uViewPos_, 1, glm::value_ptr(cam_.position()));
        glUniform1i(uTexture_, 0);
        renderScene();

        // Now render 2D overlays
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        if (gameState_ == GameState::PAUSED) {
            renderPauseMenu();
        }
        else if (gameState_ == GameState::PLAYING) {
            if (uiRenderer_) {
                renderUI();
            }
        }
        else if (gameState_ == GameState::LEVEL_TRANSITION) {
            renderLevelTransition();
        }
        else if (gameState_ == GameState::GAME_OVER ||
            gameState_ == GameState::MOUSE_WIN ||
            gameState_ == GameState::CAT_WIN) {
            renderGameOverPopup();
        }

        // Reset OpenGL state
        glEnable(GL_DEPTH_TEST);
    }
    void Game::renderScene() {
        auto setMat = [&](const glm::vec3& col, float emis, float ka, float kd, float ks, float sh) {
            glUniform3fv(uBaseColor_, 1, glm::value_ptr(col));
            glUniform1f(uEmissive_, emis);
            glUniform1f(uKa_, ka);
            glUniform1f(uKd_, kd);
            glUniform1f(uKs_, ks);
            glUniform1f(uShine_, sh);
            };

        // Ground
        {
            glm::mat4 M(1.f);
            M = glm::translate(M, { 0.f, -0.01f, 0.f });
            M = glm::scale(M, { 18.f, 0.02f, 12.f });
            glUniformMatrix4fv(uModel_, 1, GL_FALSE, glm::value_ptr(M));
            glUniform1i(uUseTexture_, 1);
            setMat({ 0.5f, 0.8f, 0.4f }, 0.0f, 0.4f, 0.9f, 0.1f, 8.0f);
            grassTex_->Bind(0);
            drawMesh(box_);
        }

        // Walls
        glUniform1i(uUseTexture_, 1);
        for (const auto& w : walls_) {
            glm::mat4 M = glm::translate(glm::mat4(1.f), w.pos);
            M = glm::scale(M, w.size);
            glUniformMatrix4fv(uModel_, 1, GL_FALSE, glm::value_ptr(M));
            setMat({ 1.0f, 0.96f, 0.75f }, 0.0f, 0.3f, 0.8f, 0.4f, 48.0f);
            stoneTex_->Bind(0);
            drawMesh(box_);
        }

        // Furniture
        for (const auto& f : furniture_) {
            glm::mat4 M = glm::translate(glm::mat4(1.f), f.pos);
            M = glm::scale(M, f.size);
            glUniformMatrix4fv(uModel_, 1, GL_FALSE, glm::value_ptr(M));
            glUniform1i(uUseTexture_, 1);
            setMat(f.color, 0.0f, 0.3f, 0.85f, 0.3f, 24.0f);
            woodTex_->Bind(0);
            drawMesh(box_);
        }

        glUniform1i(uUseTexture_, 0);

        // Cheese
        for (const auto& c : cheeses_) {
            if (c.taken) continue;
            glm::mat4 M = glm::translate(glm::mat4(1.f), c.pos + glm::vec3(0, c.bobOffset, 0));
            M = glm::rotate(M, c.rotation, glm::vec3(0, 1, 0));
            M = glm::scale(M, glm::vec3(0.45f));
            glUniformMatrix4fv(uModel_, 1, GL_FALSE, glm::value_ptr(M));
            setMat({ 1.0f, 0.95f, 0.2f }, 0.4f, 0.4f, 0.8f, 0.4f, 32.0f);
            drawMesh(cheeseModel_);
        }

        // Power-ups
        glDisable(GL_CULL_FACE);
        for (const auto& p : powerups_) {
            if (p.taken) continue;
            glm::mat4 M = glm::translate(glm::mat4(1.f), p.pos + glm::vec3(0, p.bobOffset, 0));
            M = glm::rotate(M, p.rotation, glm::vec3(0, 1, 0));
            M = glm::scale(M, glm::vec3(0.35f));
            glUniformMatrix4fv(uModel_, 1, GL_FALSE, glm::value_ptr(M));

            if (p.type == 0) {
                setMat({ 1.0f, 0.84f, 0.0f }, 1.0f, 0.3f, 0.6f, 0.9f, 96.0f);
                drawMesh(sphere_);
            }
            else if (p.type == 1) {
                setMat({ 0.0f, 1.0f, 1.0f }, 1.1f, 0.2f, 0.7f, 0.8f, 72.0f);
                drawMesh(cone_);
            }
            else {
                setMat({ 0.3f, 0.5f, 1.0f }, 1.0f, 0.3f, 0.6f, 0.9f, 80.0f);
                drawMesh(sphere_);
            }
        }
        glEnable(GL_CULL_FACE);

        // Fallback particles
        if (!particleSystem_ && !particles_.empty()) {
            glDisable(GL_CULL_FACE);
            glDepthMask(GL_FALSE);
            for (const auto& p : particles_) {
                glm::mat4 M = glm::translate(glm::mat4(1.f), p.pos);
                M = glm::scale(M, glm::vec3(p.size));
                glUniformMatrix4fv(uModel_, 1, GL_FALSE, glm::value_ptr(M));
                setMat(p.color, p.life * 2.0f, 0.1f, 0.3f, 0.2f, 8.0f);
                drawMesh(sphere_);
            }
            glDepthMask(GL_TRUE);
            glEnable(GL_CULL_FACE);
        }

        // Mouse
        {
            glm::mat4 M = glm::translate(glm::mat4(1.f), mouse_.pos);
            M = glm::rotate(M, glm::radians(mouse_.yaw), glm::vec3(0, 1, 0));
            M = glm::rotate(M, glm::radians(mouse_.pitch), glm::vec3(1, 0, 0));
            M = glm::scale(M, mouse_.size * 0.9f);
            glUniformMatrix4fv(uModel_, 1, GL_FALSE, glm::value_ptr(M));

            float glow = mouseInvincible_ ? 0.8f : 0.05f;
            glm::vec3 color = mouseInvincible_ ? glm::vec3(1.0f, 1.0f, 0.5f) : mouse_.color;

            if (mouse_.invulnerabilityTimer > 0.0f) {
                float blink = std::sin(mouse_.invulnerabilityTimer * 20.0f);
                if (blink > 0.5f) glow = 0.8f;
            }

            setMat(color, glow, 0.35f, 0.8f, 0.3f, 28.0f);
            drawMesh(mouseModel_);
        }

        // Cat
        {
            glm::mat4 M = glm::translate(glm::mat4(1.f), cat_.pos);
            M = glm::rotate(M, glm::radians(cat_.yaw), glm::vec3(0, 1, 0));
            M = glm::rotate(M, glm::radians(cat_.pitch), glm::vec3(1, 0, 0));
            M = glm::scale(M, cat_.size);
            glUniformMatrix4fv(uModel_, 1, GL_FALSE, glm::value_ptr(M));

            glm::vec3 color = catFrozen_ ? glm::vec3(0.5f, 0.7f, 1.0f) : cat_.color;
            float glow = catFrozen_ ? 0.4f : 0.05f;

            setMat(color, glow, 0.4f, 0.85f, 0.25f, 24.0f);
            drawMesh(catModel_);
        }

        // Collision effect
        if (showCollisionEffect_) {
            glDisable(GL_DEPTH_TEST);
            glm::mat4 M = glm::translate(glm::mat4(1.f), collisionPosition_);
            M = glm::scale(M, glm::vec3(2.0f + (0.5f - collisionEffectTimer_) * 4.0f));
            glUniformMatrix4fv(uModel_, 1, GL_FALSE, glm::value_ptr(M));
            float alpha = collisionEffectTimer_ / 0.5f;
            setMat({ 1.0f, 0.5f, 0.0f }, alpha * 3.0f, 0.5f, 0.5f, 0.9f, 128.0f);
            drawMesh(sphere_);
            glEnable(GL_DEPTH_TEST);
        }

        // Advanced systems
        if (particleSystem_) {
            particleSystem_->Render(cam_.view(), cam_.proj(), cam_.position());
        }

        if (lightningSystem_) {
            lightningSystem_->Render(cam_.view(), cam_.proj());
        }
    }


    // REPLACE the renderIntro() function in Game.cpp with this:

    // Replace renderIntro() in Game.cpp with this SUPER SIMPLE version:

    // COMPLETE FIXED renderIntro() - Replace in Game.cpp

    // Replace renderIntro() in Game.cpp with this version that includes text:

    void Game::renderIntro() {
        int fbWidth, fbHeight;
        glfwGetFramebufferSize(win_, &fbWidth, &fbHeight);
        glViewport(0, 0, fbWidth, fbHeight);

        glClearColor(0.15f, 0.15f, 0.25f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        if (!uiRenderer_) {
            std::cout << "PRESS 'U' TO START THE GAME!\n";
            return;
        }

        uiRenderer_->SetScreenSize(fbWidth, fbHeight);
        uiRenderer_->BeginUI();

        float w = static_cast<float>(fbWidth);
        float h = static_cast<float>(fbHeight);
        float centerX = w / 2.0f;
        float centerY = h / 2.0f;

        // Background
        uiRenderer_->RenderRect(0, 0, w, h, { 0.05f, 0.05f, 0.15f, 1.0f });

        // TOP SECTION - TITLE AREA
        float titleH = 180.0f;
        uiRenderer_->RenderRect(50, 50, w - 100, titleH, { 0.8f, 0.1f, 0.1f, 0.95f });
        uiRenderer_->RenderBorder(50, 50, w - 100, titleH, 8.0f, { 1.0f, 1.0f, 0.0f, 1.0f });
        uiRenderer_->RenderRect(60, 60, w - 120, titleH - 20, { 1.0f, 0.3f, 0.2f, 0.5f });

        // TITLE TEXT - "TOM & JERRY"
        uiRenderer_->RenderCenteredText("TOM & JERRY", 80, 6.0f, { 1.0f, 1.0f, 1.0f });

        // SUBTITLE - "CHEESE CHASE 3D"
        uiRenderer_->RenderCenteredText("CHEESE CHASE 3D", 155, 4.0f, { 1.0f, 0.9f, 0.0f });

        // MIDDLE SECTION - GAME INFO
        float infoY = 50 + titleH + 30;
        float infoH = 140.0f;
        uiRenderer_->RenderRect(50, infoY, w - 100, infoH, { 0.1f, 0.7f, 0.2f, 0.95f });
        uiRenderer_->RenderBorder(50, infoY, w - 100, infoH, 8.0f, { 1.0f, 1.0f, 1.0f, 1.0f });

        // GAME INFO TEXT
        uiRenderer_->RenderCenteredText("COLLECT ALL CHEESE", infoY + 30, 3.5f, { 1.0f, 1.0f, 1.0f });
        uiRenderer_->RenderCenteredText("AVOID TOM THE CAT", infoY + 75, 3.5f, { 1.0f, 1.0f, 1.0f });

        // CHARACTER BOXES
        float charY = infoY + infoH + 30;
        float charSize = 120.0f;

        // JERRY BOX (Left)
        float jerryX = centerX - charSize - 80;
        uiRenderer_->RenderRect(jerryX, charY, charSize, charSize, { 0.2f, 0.5f, 1.0f, 0.95f });
        uiRenderer_->RenderBorder(jerryX, charY, charSize, charSize, 6.0f, { 1.0f, 1.0f, 1.0f, 1.0f });

        // JERRY TEXT
        uiRenderer_->RenderText("JERRY", jerryX + 20, charY + 50, 3.0f, { 1.0f, 1.0f, 1.0f });

        // VS in middle
        float vsSize = 60.0f;
        float vsX = centerX - vsSize / 2.0f;
        uiRenderer_->RenderRect(vsX, charY + 30, vsSize, vsSize, { 1.0f, 0.3f, 0.3f, 0.95f });
        uiRenderer_->RenderBorder(vsX, charY + 30, vsSize, vsSize, 5.0f, { 1.0f, 1.0f, 0.0f, 1.0f });

        // VS TEXT
        uiRenderer_->RenderText("VS", vsX + 10, charY + 50, 4.0f, { 1.0f, 1.0f, 1.0f });

        // TOM BOX (Right)
        float tomX = centerX + 80;
        uiRenderer_->RenderRect(tomX, charY, charSize, charSize, { 1.0f, 0.5f, 0.1f, 0.95f });
        uiRenderer_->RenderBorder(tomX, charY, charSize, charSize, 6.0f, { 1.0f, 1.0f, 1.0f, 1.0f });

        // TOM TEXT
        uiRenderer_->RenderText("TOM", tomX + 30, charY + 50, 3.0f, { 1.0f, 1.0f, 1.0f });

        // START BUTTON - PULSING
        float pulse = (std::sin(static_cast<float>(glfwGetTime()) * 4.0f) + 1.0f) * 0.5f;

        float btnW = 600.0f;
        float btnH = 100.0f;
        float btnX = centerX - btnW / 2.0f;
        float btnY = h - 180.0f;

        // Button shadow
        uiRenderer_->RenderRect(btnX + 6, btnY + 6, btnW, btnH, { 0.0f, 0.0f, 0.0f, 0.6f });

        // Pulsing button
        float brightness = 0.6f + pulse * 0.4f;
        uiRenderer_->RenderRect(btnX, btnY, btnW, btnH,
            { brightness, brightness * 0.5f, 0.0f, 0.95f });

        float borderThickness = 6.0f + pulse * 4.0f;
        uiRenderer_->RenderBorder(btnX, btnY, btnW, btnH, borderThickness,
            { 1.0f, 1.0f, 0.0f, 0.7f + pulse * 0.3f });

        // Outer glow
        if (pulse > 0.6f) {
            float glowAlpha = (pulse - 0.6f) * 1.5f;
            float glowExpand = 15.0f;
            uiRenderer_->RenderBorder(btnX - glowExpand, btnY - glowExpand,
                btnW + glowExpand * 2, btnH + glowExpand * 2, 4.0f,
                { 1.0f, 0.8f, 0.0f, glowAlpha * 0.5f });
        }

        // BUTTON TEXT - "PRESS U TO START"
        uiRenderer_->RenderCenteredText("PRESS U TO START", btnY + 35, 4.5f, { 1.0f, 1.0f, 1.0f });

        // Corner decorations
        float cornerSize = 50.0f;
        uiRenderer_->RenderRect(10, 10, cornerSize, cornerSize, { 1.0f, 0.2f, 0.4f, 0.8f });
        uiRenderer_->RenderRect(w - cornerSize - 10, 10, cornerSize, cornerSize, { 0.2f, 1.0f, 0.4f, 0.8f });
        uiRenderer_->RenderRect(10, h - cornerSize - 10, cornerSize, cornerSize, { 0.2f, 0.4f, 1.0f, 0.8f });
        uiRenderer_->RenderRect(w - cornerSize - 10, h - cornerSize - 10, cornerSize, cornerSize, { 1.0f, 1.0f, 0.2f, 0.8f });

        uiRenderer_->EndUI();
        glEnable(GL_DEPTH_TEST);
    }

    void Game::renderUI() {
        if (!uiRenderer_) return;

        uiRenderer_->BeginUI();

        // Health bar
        float healthPercent = (float)mouse_.lives / 3.0f;
        uiRenderer_->RenderHealthBar(20, 20, 200, 30, healthPercent, { 0.2f, 1.0f, 0.2f });

        // Score display
        float scoreWidth = 180.0f;
        uiRenderer_->RenderRect(static_cast<float>(width_) - scoreWidth - 20.0f, 20, scoreWidth, 50, { 0.1f, 0.1f, 0.1f, 0.8f });
        uiRenderer_->RenderBorder(static_cast<float>(width_) - scoreWidth - 20.0f, 20, scoreWidth, 50, 2, { 1.0f, 0.84f, 0.0f, 1.0f });

        // Level display
        uiRenderer_->RenderRect(static_cast<float>(width_) / 2.0f - 100.0f, 20, 200, 50, { 0.1f, 0.1f, 0.1f, 0.8f });
        uiRenderer_->RenderBorder(static_cast<float>(width_) / 2.0f - 100.0f, 20, 200, 50, 2, { 0.3f, 0.7f, 1.0f, 1.0f });

        // Cheese collection bar
        float cheeseBarWidth = 180.0f;
        float cheesePercent = (float)collected_ / (float)totalCheese_;
        uiRenderer_->RenderRect(20, 70, cheeseBarWidth, 40, { 0.1f, 0.1f, 0.1f, 0.8f });
        uiRenderer_->RenderRect(24, 74, (cheeseBarWidth - 8.0f) * cheesePercent, 32, { 1.0f, 0.95f, 0.2f, 0.9f });
        uiRenderer_->RenderBorder(20, 70, cheeseBarWidth, 40, 2, { 1.0f, 0.95f, 0.2f, 1.0f });

        // Power-up indicator
        if (currentPowerUp_ >= 0) {
            glm::vec4 powerColor;

            if (currentPowerUp_ == 0) {
                powerColor = { 1.0f, 0.84f, 0.0f, 0.9f };
            }
            else if (currentPowerUp_ == 1) {
                powerColor = { 0.0f, 1.0f, 1.0f, 0.9f };
            }
            else {
                powerColor = { 0.3f, 0.5f, 1.0f, 0.9f };
            }

            float powerBarWidth = 200.0f * (powerUpTimer_ / 5.0f);
            float powerBarY = static_cast<float>(height_) - 70.0f;

            uiRenderer_->RenderRect(static_cast<float>(width_) / 2.0f - 100.0f, powerBarY, 200, 50, { 0.1f, 0.1f, 0.1f, 0.9f });
            uiRenderer_->RenderRect(static_cast<float>(width_) / 2.0f - 96.0f, powerBarY + 4.0f, powerBarWidth * 0.96f, 42, powerColor);
            uiRenderer_->RenderBorder(static_cast<float>(width_) / 2.0f - 100.0f, powerBarY, 200, 50, 3, { 1, 1, 1, 1 });
        }

        // Time remaining bar
        float timeLeft = levelTimeLimit_ - levelTime_;
        float timePercent = timeLeft / levelTimeLimit_;
        glm::vec3 timeColor = timePercent > 0.3f ? glm::vec3(0.2f, 1.0f, 0.2f) :
            timePercent > 0.1f ? glm::vec3(1.0f, 1.0f, 0.2f) :
            glm::vec3(1.0f, 0.2f, 0.2f);

        uiRenderer_->RenderHealthBar(20, static_cast<float>(height_) - 60.0f, 250, 35, timePercent, timeColor);

        uiRenderer_->EndUI();
    }


    void Game::renderLevelTransition() {
        if (!uiRenderer_) return;

        // Force correct OpenGL state for 2D rendering
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        int fbWidth, fbHeight;
        glfwGetFramebufferSize(win_, &fbWidth, &fbHeight);
        float fWidth = static_cast<float>(fbWidth);
        float fHeight = static_cast<float>(fbHeight);

        uiRenderer_->SetScreenSize(fbWidth, fbHeight);
        uiRenderer_->BeginUI();

        // Flash effect based on transition timer
        float flashAlpha = transitionTimer_ / 3.0f;
        uiRenderer_->RenderRect(0, 0, fWidth, fHeight, { 1.0f, 1.0f, 1.0f, flashAlpha * 0.3f });

        // Semi-transparent overlay
        uiRenderer_->RenderRect(0, 0, fWidth, fHeight, { 0.0f, 0.0f, 0.0f, 0.7f });

        // Main celebration box
        float boxW = 800.0f;
        float boxH = 600.0f;
        float boxX = fWidth / 2.0f - boxW / 2.0f;
        float boxY = fHeight / 2.0f - boxH / 2.0f;

        // Pulsing animation
        float pulse = (std::sin((3.0f - transitionTimer_) * 6.0f) + 1.0f) * 0.5f;
        float scale = 1.0f + pulse * 0.05f;

        boxW *= scale;
        boxH *= scale;
        boxX = fWidth / 2.0f - boxW / 2.0f;
        boxY = fHeight / 2.0f - boxH / 2.0f;

        // Drop shadow
        uiRenderer_->RenderRect(boxX + 15, boxY + 15, boxW, boxH, { 0.0f, 0.0f, 0.0f, 0.8f });

        // Gradient background - Green for success
        uiRenderer_->RenderRect(boxX, boxY, boxW, boxH, { 0.1f, 0.7f, 0.2f, 0.98f });
        uiRenderer_->RenderRect(boxX, boxY, boxW, boxH / 2, { 0.2f, 0.9f, 0.3f, 0.5f });

        // Animated border
        float borderThickness = 10.0f + pulse * 5.0f;
        uiRenderer_->RenderBorder(boxX, boxY, boxW, boxH, borderThickness,
            { 1.0f, 0.85f, 0.0f, 0.8f + pulse * 0.2f });

        // Inner glow
        uiRenderer_->RenderRect(boxX + 30, boxY + 30, boxW - 60, boxH - 60,
            { 1.0f, 1.0f, 1.0f, 0.15f });

        // LEVEL COMPLETE text
        float titleY = boxY + 80;
        uiRenderer_->RenderCenteredText("LEVEL COMPLETE!", titleY, 8.0f, glm::vec3(1.0f, 1.0f, 1.0f));

        // Stars decoration
        float starY = titleY + 70;
        uiRenderer_->RenderCenteredText("* * * * *", starY, 5.0f, glm::vec3(1.0f, 0.85f, 0.0f));

        // Level advancement
        float levelY = starY + 70;
        std::string levelText = "ADVANCING TO LEVEL " + std::to_string(level_);
        uiRenderer_->RenderCenteredText(levelText, levelY, 5.0f, glm::vec3(1.0f, 1.0f, 0.5f));

        // Stats panel
        float statsY = levelY + 90;
        float statsH = 180;
        uiRenderer_->RenderRect(boxX + 80, statsY, boxW - 160, statsH,
            { 0.0f, 0.0f, 0.0f, 0.6f });
        uiRenderer_->RenderBorder(boxX + 80, statsY, boxW - 160, statsH, 4,
            { 1.0f, 1.0f, 1.0f, 0.7f });

        // Display bonuses
        int timeBonus = (int)(levelTimeLimit_ - levelTime_) * 10;

        std::string scoreText = "SCORE " + std::to_string(score_);
        uiRenderer_->RenderCenteredText(scoreText, statsY + 30, 4.0f, glm::vec3(1.0f, 1.0f, 1.0f));

        std::string timeBonusText = "TIME BONUS +" + std::to_string(timeBonus);
        uiRenderer_->RenderCenteredText(timeBonusText, statsY + 75, 3.5f, glm::vec3(0.5f, 1.0f, 0.5f));

        uiRenderer_->RenderCenteredText("LEVEL BONUS +500", statsY + 115, 3.5f, glm::vec3(0.5f, 1.0f, 0.5f));

        // Get Ready message
        float readyY = boxY + boxH - 100;
        if (transitionTimer_ < 1.5f) {
            uiRenderer_->RenderCenteredText("GET READY!", readyY, 5.0f, glm::vec3(1.0f, 1.0f, 1.0f));
        }

        uiRenderer_->EndUI();
    }



    void Game::triggerVictoryCelebration() {
        // Massive particle burst
        if (particleSystem_) {
            // Gold particles
            for (int i = 0; i < 5; ++i) {
                glm::vec3 spawnPos = mouse_.pos + glm::vec3(
                    (rand() % 100 - 50) / 25.0f,
                    2.0f,
                    (rand() % 100 - 50) / 25.0f
                );
                particleSystem_->CreateExplosion(spawnPos, glm::vec4(1.0f, 0.84f, 0.0f, 1.0f), 50);
            }

            // Rainbow particles
            glm::vec4 colors[] = {
                glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), // Red
                glm::vec4(1.0f, 0.5f, 0.0f, 1.0f), // Orange
                glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), // Yellow
                glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), // Green
                glm::vec4(0.0f, 0.5f, 1.0f, 1.0f), // Blue
                glm::vec4(0.5f, 0.0f, 1.0f, 1.0f)  // Purple
            };

            for (int i = 0; i < 6; ++i) {
                glm::vec3 offset(
                    std::cos(i * 3.14159f / 3.0f) * 3.0f,
                    3.0f,
                    std::sin(i * 3.14159f / 3.0f) * 3.0f
                );
                particleSystem_->CreateExplosion(mouse_.pos + offset, colors[i], 40);
            }
        }

        // Lightning celebration
        if (lightningSystem_) {
            lightningIntensity_ = 1.0f;

            // Create lightning bolts in a circle
            for (int i = 0; i < 12; ++i) {
                float angle = (i / 12.0f) * 2.0f * 3.14159f;
                glm::vec3 start(
                    std::cos(angle) * 8.0f,
                    10.0f,
                    std::sin(angle) * 8.0f
                );
                glm::vec3 end = mouse_.pos + glm::vec3(0, 1, 0);
                lightningSystem_->TriggerLightning(start, end);
            }

            // Vertical lightning bolts
            for (int i = 0; i < 8; ++i) {
                glm::vec3 offset(
                    (rand() % 200 - 100) / 20.0f,
                    0,
                    (rand() % 200 - 100) / 20.0f
                );
                lightningSystem_->TriggerLightning(
                    mouse_.pos + offset + glm::vec3(0, 8, 0),
                    mouse_.pos + offset
                );
            }
        }

        screenShakeAmount_ = 0.5f;
    }



    void Game::renderPauseMenu() {
        if (!uiRenderer_) return;

        uiRenderer_->BeginUI();

        float fWidth = static_cast<float>(width_);
        float fHeight = static_cast<float>(height_);

        uiRenderer_->RenderRect(0, 0, fWidth, fHeight, { 0.0f, 0.0f, 0.0f, 0.6f });

        uiRenderer_->RenderRect(fWidth / 2.0f - 250.0f, fHeight / 2.0f - 150.0f, 500, 300, { 0.2f, 0.2f, 0.2f, 0.95f });
        uiRenderer_->RenderBorder(fWidth / 2.0f - 250.0f, fHeight / 2.0f - 150.0f, 500, 300, 4, { 1.0f, 1.0f, 1.0f, 1.0f });

        uiRenderer_->RenderRect(fWidth / 2.0f - 150.0f, fHeight / 2.0f - 100.0f, 300, 80, { 1.0f, 0.5f, 0.0f, 0.9f });

        uiRenderer_->EndUI();
    }

    // Replace renderGameOverPopup() in Game.cpp with this enhanced version:

    // SIMPLIFIED VERSION FOR TESTING - Replace renderGameOverPopup() in Game.cpp:

    // REPLACE your renderGameOverPopup() function in Game.cpp with this fixed version:

    void Game::renderGameOverPopup() {
        if (!uiRenderer_) {
            std::cout << "GAME OVER - uiRenderer_ is NULL!\n";
            return;
        }

        // DEBUG: Print what we're trying to display
        std::cout << "Rendering score: " << score_ << "\n";
        std::string scoreText = "SCORE ";
        scoreText += std::to_string(score_);
        std::cout << "Score text string: '" << scoreText << "'\n";
        std::cout << "String length: " << scoreText.length() << "\n";
        for (size_t i = 0; i < scoreText.length(); ++i) {
            std::cout << "  [" << i << "] = '" << scoreText[i] << "' (ASCII " << (int)scoreText[i] << ")\n";
        }

        // Force correct OpenGL state for 2D rendering
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Get window size
        int fbWidth, fbHeight;
        glfwGetFramebufferSize(win_, &fbWidth, &fbHeight);
        float fWidth = static_cast<float>(fbWidth);
        float fHeight = static_cast<float>(fbHeight);

        uiRenderer_->SetScreenSize(fbWidth, fbHeight);
        uiRenderer_->BeginUI();

        // Semi-transparent dark overlay
        uiRenderer_->RenderRect(0, 0, fWidth, fHeight, { 0.0f, 0.0f, 0.0f, 0.85f });

        // Popup dimensions
        float boxW = 700.0f;
        float boxH = 500.0f;
        float boxX = fWidth / 2.0f - boxW / 2.0f;
        float boxY = fHeight / 2.0f - boxH / 2.0f;

        // Drop shadow
        uiRenderer_->RenderRect(boxX + 10, boxY + 10, boxW, boxH, { 0.0f, 0.0f, 0.0f, 0.7f });

        // Main box background - Red for game over
        uiRenderer_->RenderRect(boxX, boxY, boxW, boxH, { 0.8f, 0.2f, 0.2f, 0.98f });

        // Border with animation
        float pulse = (std::sin(gameOverTimer_ * 3.0f) + 1.0f) * 0.5f;
        float borderThickness = 8.0f + pulse * 4.0f;
        uiRenderer_->RenderBorder(boxX, boxY, boxW, boxH, borderThickness,
            { 1.0f, 1.0f, 0.0f, 0.7f + pulse * 0.3f });

        // Inner decorative border
        uiRenderer_->RenderRect(boxX + 20, boxY + 20, boxW - 40, boxH - 40,
            { 1.0f, 1.0f, 1.0f, 0.15f });

        // GAME OVER text
        float titleY = boxY + 70;
        uiRenderer_->RenderCenteredText("GAME OVER", titleY, 7.0f, glm::vec3(1.0f, 1.0f, 1.0f));

        // Stats panel
        float statsY = boxY + 200;
        float statsH = 160;
        uiRenderer_->RenderRect(boxX + 50, statsY, boxW - 100, statsH,
            { 0.0f, 0.0f, 0.0f, 0.5f });
        uiRenderer_->RenderBorder(boxX + 50, statsY, boxW - 100, statsH, 3,
            { 1.0f, 1.0f, 1.0f, 0.6f });

        // TEST: Try rendering just numbers
        uiRenderer_->RenderCenteredText("0123456789", statsY + 30, 4.5f, glm::vec3(1.0f, 1.0f, 1.0f));

        // Display score with proper string construction
        std::stringstream ss;
        ss << "SCORE " << score_;
        std::string finalScoreText = ss.str();
        std::cout << "Final score text: '" << finalScoreText << "'\n";
        uiRenderer_->RenderCenteredText(finalScoreText, statsY + 75, 3.5f, glm::vec3(1.0f, 0.8f, 0.3f));

        // Restart button with pulse
        float btnY = boxY + boxH - 110;
        float btnW = 500;
        float btnH = 70;
        float btnX = fWidth / 2.0f - btnW / 2.0f;

        float brightness = 0.6f + pulse * 0.4f;
        uiRenderer_->RenderRect(btnX, btnY, btnW, btnH,
            { brightness, brightness * 0.5f, 0.0f, 0.95f });
        uiRenderer_->RenderBorder(btnX, btnY, btnW, btnH, 5,
            { 1.0f, 1.0f, 0.0f, 0.8f + pulse * 0.2f });

        uiRenderer_->RenderCenteredText("PRESS R TO RESTART", btnY + 25, 4.0f, glm::vec3(1.0f, 1.0f, 1.0f));

        uiRenderer_->EndUI();
    }

