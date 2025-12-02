// Game.cpp - Part 1 of 3: Initialization and Setup
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

// Utility functions
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
    std::cout << "    TOM & JERRY: CHEESE CHASE 3D - ULTIMATE EDITION            \n";
    std::cout << "================================================================\n";
    std::cout << " OBJECTIVE:                                                    \n";
    std::cout << "   Control Jerry (Mouse) and collect ALL cheese!               \n";
    std::cout << "   Avoid Tom (Cat) who hunts with smart AI!                    \n";
    std::cout << "                                                               \n";
    std::cout << " CONTROLS:                                                     \n";
    std::cout << "   MENU:          SPACE - Start Game                          \n";
    std::cout << "   Jerry:         W/A/S/D OR ARROW KEYS - Move                \n";
    std::cout << "   Camera:        Q/E - Rotate | Z/X - Height                 \n";
    std::cout << "   Game:          P - Pause | R - Reset | ESC - Quit          \n";
    std::cout << "   Sound:         M - Toggle Sound ON/OFF                     \n";
    std::cout << "                                                               \n";
    std::cout << " POWER-UPS (Last 5 seconds):                                  \n";
    std::cout << "   Gold Sphere  - SHIELD (Invincible)                         \n";
    std::cout << "   Cyan Cone    - SPEED BOOST (2x speed)                      \n";
    std::cout << "   Blue Sphere  - FREEZE TOM (3 seconds)                      \n";
    std::cout << "                                                               \n";
    std::cout << " LIVES & SCORING:                                              \n";
    std::cout << "   Start with 3 lives                                          \n";
    std::cout << "   Cheese: +100 points each                                    \n";
    std::cout << "   Level complete: +500 bonus                                  \n";
    std::cout << "   Time bonus: remaining seconds x 10                          \n";
    std::cout << "                                                               \n";
    std::cout << " AUDIO FEEDBACK:                                               \n";
    std::cout << "   Collect cheese, power-ups, win/lose sounds                 \n";
    std::cout << "   Chase alerts and special effect sounds                     \n";
    std::cout << "                                                               \n";
    std::cout << " FEATURES:                                                     \n";
    std::cout << "   3 progressively harder levels                               \n";
    std::cout << "   Advanced AI with patrol/chase behavior                      \n";
    std::cout << "   Spectacular particle effects                                \n";
    std::cout << "   Blinn-Phong lighting system                                 \n";
    std::cout << "   Dynamic camera with smooth tracking                         \n";
    std::cout << "   Sound effects for all game events                           \n";
    std::cout << "================================================================\n";
    std::cout << "\n";
}

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
    initSceneGraph();
    resetWorld();
    loop();

    glfwDestroyWindow(win_);
    glfwTerminate();
}

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

    win_ = glfwCreateWindow(width_, height_, "Tom & Jerry: Cheese Chase 3D", nullptr, nullptr);
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

    glm::vec3 L1pos(12.f, 18.f, 10.f);
    glm::vec3 L1col(1.0f, 0.95f, 0.85f);
    glm::vec3 L2pos(-10.f, 14.f, -8.f);
    glm::vec3 L2col(0.5f, 0.6f, 0.8f);

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

void Game::initSceneGraph() {
    std::cout << "Building scene graph...\n";
    sceneRoot_ = std::make_shared<Node>("SceneRoot");

    cannonRoot_ = std::make_shared<Node>("CannonRoot");
    cannonRoot_->SetPosition({ -6.5f, 0.0f, -4.0f });
    sceneRoot_->AddChild(cannonRoot_);

    auto cannonBase = std::make_shared<Node>("CannonBase");
    cannonBase->SetPosition({ 0.0f, 0.8f, 0.0f });
    cannonBase->SetScale({ 0.8f, 1.6f, 0.8f });
    cannonBase->SetRenderCallback([this](const glm::mat4& transform) {
        glUseProgram(prog_);
        glUniformMatrix4fv(uModel_, 1, GL_FALSE, glm::value_ptr(transform));
        glUniform1i(uUseTexture_, 1);
        glUniform1f(uKa_, 0.3f);
        glUniform1f(uKd_, 0.7f);
        glUniform1f(uKs_, 0.3f);
        glUniform1f(uShine_, 32.0f);
        glUniform1f(uEmissive_, 0.0f);
        stoneTex_->Bind(0);
        drawMesh(cyl_);
        });
    cannonRoot_->AddChild(cannonBase);

    cannonBarrel_ = std::make_shared<Node>("BarrelHinge");
    cannonBarrel_->SetPosition({ 0.0f, 1.6f, 0.0f });
    cannonRoot_->AddChild(cannonBarrel_);

    auto barrel = std::make_shared<Node>("Barrel");
    barrel->SetPosition({ 1.2f, 0.0f, 0.0f });
    barrel->SetRotation(90.0f, { 0, 0, 1 });
    barrel->SetScale({ 0.15f, 1.8f, 0.15f });
    barrel->SetRenderCallback([this](const glm::mat4& transform) {
        glUseProgram(prog_);
        glUniformMatrix4fv(uModel_, 1, GL_FALSE, glm::value_ptr(transform));
        glUniform1i(uUseTexture_, 1);
        glUniform1f(uKa_, 0.2f);
        glUniform1f(uKd_, 0.7f);
        glUniform1f(uKs_, 0.8f);
        glUniform1f(uShine_, 96.0f);
        glUniform1f(uEmissive_, 0.0f);
        metalTex_->Bind(0);
        drawMesh(cyl_);
        });
    cannonBarrel_->AddChild(barrel);

    std::cout << "Scene graph created!\n";
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
    mouse_.speed = 8.0f;  // Increased from 4.2f for better responsiveness
    mouse_.lives = 3;

    cat_ = {};
    cat_.pos = { 3.5f, 0.4f, 2.0f };
    cat_.size = { 1.0f, 1.2f, 1.0f };
    cat_.color = { 1.0f, 0.63f, 0.35f };
    cat_.speed = 6.0f + level_ * 0.3f; // Reduced base speed from 3.0f
    catState_ = CatState::PATROL;
    catTarget_ = cat_.pos;

    walls_.clear();
    auto wall = [&](float x, float z, float sx, float sz) {
        Entity w;
        w.pos = { x, 0.75f, z };
        w.size = { sx, 1.5f, sz };
        w.color = { 1.0f, 0.96f, 0.75f };
        walls_.push_back(w);
        };
    const float W = 18.f;
    const float D = 12.f;
    wall(0.f, -D * 0.5f, W, 0.8f);
    wall(0.f, D * 0.5f, W, 0.8f);
    wall(-W * 0.5f, 0.f, 0.8f, D);
    wall(W * 0.5f, 0.f, 0.8f, D);

    furniture_.clear();
    auto addF = [&](const glm::vec3& p, const glm::vec3& s, const glm::vec3& c) {
        Entity f;
        f.pos = p;
        f.size = s;
        f.color = c;
        f.dynamic = true;
        furniture_.push_back(f);
        };
    addF({ -4.0f, 0.5f, -1.5f }, { 2.0f, 1.0f, 1.2f }, { 0.72f, 0.52f, 0.36f });
    addF({ 0.0f, 0.6f, 0.0f }, { 3.0f, 1.2f, 1.0f }, { 0.86f, 0.57f, 0.40f });
    addF({ 2.0f, 0.5f, 2.5f }, { 1.7f, 1.0f, 1.5f }, { 0.45f, 0.64f, 0.86f });
    addF({ -2.5f, 0.5f, 3.0f }, { 1.5f, 1.0f, 1.0f }, { 0.65f, 0.45f, 0.35f });

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
    // Print level start message
        if (level_ > 1) {
            std::cout << "\n";
            std::cout << "╔════════════════════════════════════════════╗\n";
            std::cout << "║          🚀 LEVEL " << level_ << " STARTED! 🚀          ║\n";
            std::cout << "╔════════════════════════════════════════════╗\n";
            std::cout << "║  Cheese to collect: " << totalCheese_ << "                     ║\n";
            std::cout << "║  Tom's speed:       " << cat_.speed << "                  ║\n";
            std::cout << "║  Time limit:        " << (int)levelTimeLimit_ << " seconds           ║\n";
            std::cout << "╚════════════════════════════════════════════╝\n";
            std::cout << "\n";
        }
}

void Game::startGame() {
    gameState_ = GameState::PLAYING;
    level_ = 1;
    score_ = 0;
    gameTime_ = 0.0f;

    cameraAngle_ = 0.0f;
    cameraHeight_ = 20.0f;
    cameraDistance_ = 22.0f;

    resetWorld();

    // Start background music
    if (soundSystem_ && soundEnabled_) {
        soundSystem_->PlayBackgroundMusic();
    }
}
// Game.cpp - Part 2 of 3: Game Logic, AI, Updates

void Game::nextLevel() {
    int timeBonus = (int)(levelTimeLimit_ - levelTime_) * 10;
    if (timeBonus > 0) score_ += timeBonus;
    score_ += 500;

    level_++;
    transitionTimer_ = 3.0f;
    gameState_ = GameState::LEVEL_TRANSITION;

    spawnParticles(mouse_.pos, glm::vec3(0.0f, 1.0f, 0.0f), 100);

    // Terminal message
    std::cout << "\n";
    std::cout << "================================================\n";
    std::cout << "         🎉 LEVEL " << (level_ - 1) << " COMPLETE! 🎉\n";
    std::cout << "================================================\n";
    std::cout << " Time Bonus:     +" << timeBonus << " points\n";
    std::cout << " Level Bonus:    +500 points\n";
    std::cout << " Total Score:    " << score_ << "\n";
    std::cout << "------------------------------------------------\n";
    std::cout << "         ADVANCING TO LEVEL " << level_ << "...\n";
    std::cout << "================================================\n";
    std::cout << "\n";

    if (soundSystem_ && soundEnabled_) {
        soundSystem_->Play(SoundSystem::LEVEL_COMPLETE);
    }
}
void Game::loseLife() {
    mouse_.lives--;
    mouse_.invulnerabilityTimer = 2.0f;

    spawnParticles(mouse_.pos, glm::vec3(1.0f, 0.0f, 0.0f), 50);

    if (soundSystem_ && soundEnabled_) {
        soundSystem_->Play(SoundSystem::LOSE_LIFE);
    }

    if (mouse_.lives <= 0) {
        gameState_ = GameState::CAT_WIN;
        if (soundSystem_ && soundEnabled_) {
            soundSystem_->Play(SoundSystem::GAME_LOSE);
        }
    }
    else {
        mouse_.pos = { -4.f, 0.4f, -2.f };
    }
}

void Game::updateAI(float dt) {
    if (catFrozen_) {
        catState_ = CatState::CONFUSED;
        return;
    }

    aiUpdateTimer_ += dt;

    float distToMouse = glm::length(mouse_.pos - cat_.pos);

    // Update AI target every 0.3 seconds
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

            if (distToMouse < 8.0f) {
                catState_ = CatState::CHASE;
                if (soundSystem_ && soundEnabled_) {
                    soundSystem_->Play(SoundSystem::CAT_CHASE);
                }
            }
            break;
        }

        case CatState::CHASE: {
            catTarget_ = mouse_.pos;
            if (distToMouse > 12.0f) {
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

    // Smooth cat movement
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
        std::cout << "POWER-UP: SHIELD ACTIVATED!\n";
        if (soundSystem_ && soundEnabled_) {
            soundSystem_->Play(SoundSystem::SHIELD_ACTIVE);
        }
        break;
    case 1:
        mouseSpeedBoost_ = true;
        std::cout << "POWER-UP: SPEED BOOST!\n";
        if (soundSystem_ && soundEnabled_) {
            soundSystem_->Play(SoundSystem::SPEED_BOOST);
        }
        break;
    case 2:
        catFrozen_ = true;
        catState_ = CatState::CONFUSED;
        powerUpTimer_ = 3.0f;
        std::cout << "POWER-UP: TOM FROZEN!\n";
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

void Game::checkWinConditions() {
    if (collected_ >= totalCheese_) {
        if (level_ >= 3) {
            gameState_ = GameState::MOUSE_WIN;
            if (soundSystem_ && soundEnabled_) {
                soundSystem_->Play(SoundSystem::GAME_WIN);
            }
        }
        else {
            nextLevel();
        }
    }

    if (levelTime_ >= levelTimeLimit_) {
        gameState_ = GameState::CAT_WIN;
        if (soundSystem_ && soundEnabled_) {
            soundSystem_->Play(SoundSystem::GAME_LOSE);
        }
    }
}

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

    if (keys_[GLFW_KEY_R]) {
        startGame();
        keys_[GLFW_KEY_R] = false;
    }

    if (keys_[GLFW_KEY_M]) {
        soundEnabled_ = !soundEnabled_;
        std::cout << "Sound: " << (soundEnabled_ ? "ON" : "OFF") << "\n";
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

    glm::vec3 camPos(
        cameraDistance_ * std::cos(cameraAngle_),
        cameraHeight_,
        cameraDistance_ * std::sin(cameraAngle_)
    );
    cam_.setPosition(camPos);
    cam_.setTarget(glm::vec3(0.f, 0.f, 0.f));

    if (sceneRoot_) {
        sceneRoot_->Update(dt);
    }
    updateCannon(dt);

    if (particleSystem_) particleSystem_->Update(dt);
    if (lightningSystem_) lightningSystem_->Update(dt);

    switch (gameState_) {
    case GameState::MENU:
        updateMenu(dt);
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

    title += " | Sound:" + std::string(soundEnabled_ ? "ON" : "OFF");

    glfwSetWindowTitle(win_, title.c_str());
}

void Game::updateMenu(float dt) {
    if (keys_[GLFW_KEY_SPACE]) {
        startGame();
        keys_[GLFW_KEY_SPACE] = false;
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

    // Mouse movement
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

    updateAI(dt);
    updatePhysics(dt);
    updatePowerUps(dt);

    // Cheese animation
    for (auto& c : cheeses_) {
        if (!c.taken) {
            c.rotation += dt * 1.5f;
            c.bobOffset = std::sin(gameTime_ * 2.0f + c.rotation) * 0.08f;
        }
    }

    // Particle updates
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
    if (keys_[GLFW_KEY_P]) {
        gameState_ = GameState::PLAYING;
        keys_[GLFW_KEY_P] = false;
    }
}

void Game::updateCannon(float dt) {
    cannonRotation_ += 15.0f * dt;
    if (cannonRotation_ > 360.0f) cannonRotation_ -= 360.0f;
    cannonRoot_->SetRotation(cannonRotation_, { 0, 1, 0 });

    static float tiltDir = 1.0f;
    cannonTilt_ += 10.0f * dt * tiltDir;
    if (cannonTilt_ > 20.0f) { cannonTilt_ = 20.0f; tiltDir = -1.0f; }
    if (cannonTilt_ < -10.0f) { cannonTilt_ = -10.0f; tiltDir = 1.0f; }
    cannonBarrel_->SetRotation(cannonTilt_, { 0, 0, 1 });
}

void Game::updatePhysics(float dt) {
    auto resolveWith = [&](std::vector<Entity>& blocks, Player& p) {
        for (const auto& w : blocks) {
            AABB a = p.bounds();
            AABB b = w.bounds();
            if (intersects(a, b)) {
                glm::vec3 overlap = overlapVec(a, b);
                p.pos += overlap;
            }
        }
    };

    resolveWith(walls_, mouse_);
    resolveWith(walls_, cat_);

    // Furniture collision with balanced pushback
    for (auto& f : furniture_) {
        {
            AABB a = mouse_.bounds();
            AABB b = f.bounds();
            if (intersects(a, b)) {
                glm::vec3 o = overlapVec(a, b);
                if (f.dynamic) f.pos -= 0.5f * o;
                mouse_.pos += 0.5f * o;
            }
        }

        {
            AABB a = cat_.bounds();
            AABB b = f.bounds();
            if (intersects(a, b)) {
                glm::vec3 o = overlapVec(a, b);
                if (f.dynamic) f.pos -= 0.5f * o;
                cat_.pos += 0.5f * o;
            }
        }

        for (const auto& w : walls_) {
            AABB fb = f.bounds();
            AABB wb = w.bounds();
            if (intersects(fb, wb)) {
                f.pos += overlapVec(fb, wb);
            }
        }
    }
}bool Game::intersects(const AABB& a, const AABB& b) {
    return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
        (a.min.y <= b.max.y && a.max.y >= b.min.y) &&
        (a.min.z <= b.max.z && a.max.z >= b.min.z);
}

glm::vec3 Game::overlapVec(const AABB& a, const AABB& b) {
    float x1 = b.max.x - a.min.x, x2 = a.max.x - b.min.x;
    float y1 = b.max.y - a.min.y, y2 = a.max.y - b.min.y;
    float z1 = b.max.z - a.min.z, z2 = a.max.z - b.min.z;

    if (x1 <= 0.f || x2 <= 0.f || y1 <= 0.f || y2 <= 0.f || z1 <= 0.f || z2 <= 0.f)
        return glm::vec3(0);

    float mx = (x1 < x2) ? x1 : x2;
    float my = (y1 < y2) ? y1 : y2;
    float mz = (z1 < z2) ? z1 : z2;

    if (mx < my && mx < mz) return glm::vec3((x1 < x2 ? -x1 : x2), 0, 0);
    if (my < mz) return glm::vec3(0, (y1 < y2 ? -y1 : y2), 0);
    return glm::vec3(0, 0, (z1 < z2 ? -z1 : z2));
}
// Game.cpp - Part 3 of 3: Rendering

void Game::render() {
    int W, H;
    glfwGetFramebufferSize(win_, &W, &H);
    glViewport(0, 0, W, H);

    glClearColor(0.52f, 0.76f, 0.92f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(prog_);

    glm::mat4 V = cam_.view();
    glm::mat4 P = cam_.proj();

    glUniformMatrix4fv(uView_, 1, GL_FALSE, glm::value_ptr(V));
    glUniformMatrix4fv(uProj_, 1, GL_FALSE, glm::value_ptr(P));
    glUniform3fv(uViewPos_, 1, glm::value_ptr(cam_.position()));
    glUniform1i(uTexture_, 0);

    renderScene();

    if (gameState_ == GameState::MENU) {
        renderMenu();
    }
    else if (gameState_ == GameState::PAUSED) {
        renderPauseMenu();
    }
    else if (gameState_ == GameState::MOUSE_WIN ||
        gameState_ == GameState::CAT_WIN) {
        renderGameOver();
    }

    if (uiRenderer_) {
        renderUI();
    }
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

    if (sceneRoot_) {
        sceneRoot_->Render();
    }

    {
        glm::mat4 M(1.f);
        M = glm::translate(M, { 0.f, -0.01f, 0.f });
        M = glm::scale(M, { 18.f, 0.02f, 12.f });
        glUniformMatrix4fv(uModel_, 1, GL_FALSE, glm::value_ptr(M));
        glUniform1i(uUseTexture_, 1);
        setMat({ 0.5f, 0.8f, 0.4f }, 0.0f, 0.3f, 0.8f, 0.05f, 4.0f);
        grassTex_->Bind(0);
        drawMesh(box_);
    }

    glUniform1i(uUseTexture_, 1);
    for (const auto& w : walls_) {
        glm::mat4 M = glm::translate(glm::mat4(1.f), w.pos);
        M = glm::scale(M, w.size);
        glUniformMatrix4fv(uModel_, 1, GL_FALSE, glm::value_ptr(M));
        setMat({ 1.0f, 0.96f, 0.75f }, 0.0f, 0.2f, 0.7f, 0.3f, 32.0f);
        stoneTex_->Bind(0);
        drawMesh(box_);
    }

    for (const auto& f : furniture_) {
        glm::mat4 M = glm::translate(glm::mat4(1.f), f.pos);
        M = glm::scale(M, f.size);
        glUniformMatrix4fv(uModel_, 1, GL_FALSE, glm::value_ptr(M));
        glUniform1i(uUseTexture_, 1);
        setMat(f.color, 0.0f, 0.25f, 0.75f, 0.2f, 16.0f);
        woodTex_->Bind(0);
        drawMesh(box_);
    }

    glUniform1i(uUseTexture_, 0);

    for (const auto& c : cheeses_) {
        if (c.taken) continue;
        glm::mat4 M = glm::translate(glm::mat4(1.f), c.pos + glm::vec3(0, c.bobOffset, 0));
        M = glm::rotate(M, c.rotation, glm::vec3(0, 1, 0));
        M = glm::scale(M, glm::vec3(0.45f));
        glUniformMatrix4fv(uModel_, 1, GL_FALSE, glm::value_ptr(M));
        setMat({ 1.0f, 0.95f, 0.2f }, 0.3f, 0.4f, 0.7f, 0.3f, 24.0f);
        drawMesh(cheeseModel_);
    }

    glDisable(GL_CULL_FACE);
    for (const auto& p : powerups_) {
        if (p.taken) continue;
        glm::mat4 M = glm::translate(glm::mat4(1.f), p.pos + glm::vec3(0, p.bobOffset, 0));
        M = glm::rotate(M, p.rotation, glm::vec3(0, 1, 0));
        M = glm::scale(M, glm::vec3(0.35f));
        glUniformMatrix4fv(uModel_, 1, GL_FALSE, glm::value_ptr(M));

        if (p.type == 0) {
            setMat({ 1.0f, 0.84f, 0.0f }, 0.8f, 0.3f, 0.6f, 0.8f, 64.0f);
            drawMesh(sphere_);
        }
        else if (p.type == 1) {
            setMat({ 0.0f, 1.0f, 1.0f }, 0.9f, 0.2f, 0.7f, 0.6f, 48.0f);
            drawMesh(cone_);
        }
        else {
            setMat({ 0.3f, 0.5f, 1.0f }, 0.9f, 0.3f, 0.6f, 0.7f, 56.0f);
            drawMesh(sphere_);
        }
    }
    glEnable(GL_CULL_FACE);

    if (!particleSystem_ && !particles_.empty()) {
        glDisable(GL_CULL_FACE);
        glDepthMask(GL_FALSE);
        for (const auto& p : particles_) {
            glm::mat4 M = glm::translate(glm::mat4(1.f), p.pos);
            M = glm::scale(M, glm::vec3(p.size));
            glUniformMatrix4fv(uModel_, 1, GL_FALSE, glm::value_ptr(M));
            setMat(p.color, p.life * 1.5f, 0.1f, 0.3f, 0.2f, 8.0f);
            drawMesh(sphere_);
        }
        glDepthMask(GL_TRUE);
        glEnable(GL_CULL_FACE);
    }

    {
        glm::mat4 M = glm::translate(glm::mat4(1.f), mouse_.pos);
        M = glm::scale(M, mouse_.size * 0.9f);
        glUniformMatrix4fv(uModel_, 1, GL_FALSE, glm::value_ptr(M));

        float glow = mouseInvincible_ ? 0.5f : 0.05f;
        glm::vec3 color = mouseInvincible_ ? glm::vec3(1.0f, 1.0f, 0.5f) : mouse_.color;

        if (mouse_.invulnerabilityTimer > 0.0f) {
            float blink = std::sin(mouse_.invulnerabilityTimer * 20.0f);
            if (blink > 0.5f) glow = 0.8f;
        }

        setMat(color, glow, 0.35f, 0.7f, 0.25f, 20.0f);
        drawMesh(mouseModel_);
    }

    {
        glm::mat4 M = glm::translate(glm::mat4(1.f), cat_.pos);
        M = glm::scale(M, cat_.size);
        glUniformMatrix4fv(uModel_, 1, GL_FALSE, glm::value_ptr(M));

        glm::vec3 color = catFrozen_ ? glm::vec3(0.5f, 0.7f, 1.0f) : cat_.color;
        float glow = catFrozen_ ? 0.3f : 0.05f;

        setMat(color, glow, 0.4f, 0.75f, 0.2f, 18.0f);
        drawMesh(catModel_);
    }

    if (particleSystem_) {
        particleSystem_->Render(cam_.view(), cam_.proj(), cam_.position());
    }

    if (lightningSystem_) {
        lightningSystem_->Render(cam_.view(), cam_.proj());
    }
}

void Game::renderUI() {
    if (!uiRenderer_) return;

    uiRenderer_->BeginUI();

    float healthPercent = (float)mouse_.lives / 3.0f;
    uiRenderer_->RenderHealthBar(20, 20, 200, 30, healthPercent, { 0.2f, 1.0f, 0.2f });

    float scoreWidth = 150.0f;
    uiRenderer_->RenderRect(width_ - scoreWidth - 20, 20, scoreWidth, 40,
        { 0.1f, 0.1f, 0.1f, 0.7f });

    uiRenderer_->RenderRect(width_ / 2 - 75, 20, 150, 40,
        { 0.1f, 0.1f, 0.1f, 0.7f });

    if (currentPowerUp_ >= 0) {
        glm::vec4 powerColor;
        if (currentPowerUp_ == 0) powerColor = { 1.0f, 0.84f, 0.0f, 0.8f };
        else if (currentPowerUp_ == 1) powerColor = { 0.0f, 1.0f, 1.0f, 0.8f };
        else powerColor = { 0.3f, 0.5f, 1.0f, 0.8f };

        float powerBarWidth = 150.0f * (powerUpTimer_ / 5.0f);
        uiRenderer_->RenderRect(width_ / 2 - 75, height_ - 60, powerBarWidth, 30, powerColor);
        uiRenderer_->RenderBorder(width_ / 2 - 75, height_ - 60, 150, 30, 2, { 1, 1, 1, 1 });
    }

    float timeLeft = levelTimeLimit_ - levelTime_;
    float timePercent = timeLeft / levelTimeLimit_;
    glm::vec3 timeColor = timePercent > 0.3f ? glm::vec3(0.2f, 1.0f, 0.2f) :
        glm::vec3(1.0f, 0.2f, 0.2f);
    uiRenderer_->RenderHealthBar(20, height_ - 60, 200, 30, timePercent, timeColor);

    uiRenderer_->EndUI();
}

void Game::renderMenu() {
    if (!uiRenderer_) return;

    uiRenderer_->BeginUI();

    float boxW = 700;
    float boxH = 500;
    float boxX = width_ / 2 - boxW / 2;
    float boxY = height_ / 2 - boxH / 2;

    uiRenderer_->RenderRect(boxX, boxY, boxW, boxH,
        { 0.05f, 0.05f, 0.1f, 0.95f });
    uiRenderer_->RenderBorder(boxX, boxY, boxW, boxH, 5,
        { 1.0f, 0.84f, 0.0f, 1.0f });

    uiRenderer_->RenderRect(boxX + 50, boxY + 50, boxW - 100, 80,
        { 0.8f, 0.6f, 0.0f, 1.0f });

    uiRenderer_->RenderRect(boxX + 100, boxY + 150, boxW - 200, 50,
        { 0.1f, 0.4f, 0.6f, 0.9f });

    float instY = boxY + 230;
    float instHeight = 40;
    float spacing = 50;

    uiRenderer_->RenderRect(boxX + 80, instY, 120, instHeight,
        { 0.92f, 0.92f, 1.0f, 0.8f });

    uiRenderer_->RenderRect(boxX + 220, instY, 120, instHeight,
        { 1.0f, 0.63f, 0.35f, 0.8f });

    uiRenderer_->RenderRect(boxX + 360, instY, 120, instHeight,
        { 1.0f, 0.95f, 0.2f, 0.8f });

    instY += spacing;
    uiRenderer_->RenderRect(boxX + 80, instY, 250, instHeight,
        { 0.2f, 0.6f, 0.2f, 0.8f });

    instY += spacing;
    uiRenderer_->RenderRect(boxX + 80, instY, 250, instHeight,
        { 0.6f, 0.2f, 0.2f, 0.8f });

    float pulse = (std::sin(gameTime_ * 4.0f) + 1.0f) * 0.5f;
    uiRenderer_->RenderRect(width_ / 2 - 200, boxY + boxH - 100, 400, 60,
        { pulse * 0.9f, pulse * 0.5f, 0.0f, 0.9f });
    uiRenderer_->RenderBorder(width_ / 2 - 200, boxY + boxH - 100, 400, 60, 3,
        { 1.0f, 1.0f, 1.0f, 1.0f });

    uiRenderer_->EndUI();
}

void Game::renderPauseMenu() {
    if (!uiRenderer_) return;

    uiRenderer_->BeginUI();

    uiRenderer_->RenderRect(0, 0, width_, height_, { 0.0f, 0.0f, 0.0f, 0.5f });

    uiRenderer_->RenderRect(width_ / 2 - 200, height_ / 2 - 100, 400, 200,
        { 0.2f, 0.2f, 0.2f, 0.95f });
    uiRenderer_->RenderBorder(width_ / 2 - 200, height_ / 2 - 100, 400, 200, 3,
        { 1.0f, 1.0f, 1.0f, 1.0f });

    uiRenderer_->EndUI();
}

void Game::renderGameOver() {
    if (!uiRenderer_) return;

    uiRenderer_->BeginUI();

    uiRenderer_->RenderRect(0, 0, width_, height_, { 0.0f, 0.0f, 0.0f, 0.7f });

    bool won = (gameState_ == GameState::MOUSE_WIN);
    glm::vec4 mainColor = won ?
        glm::vec4(0.0f, 0.6f, 0.0f, 0.98f) :
        glm::vec4(0.6f, 0.0f, 0.0f, 0.98f);

    glm::vec4 accentColor = won ?
        glm::vec4(0.2f, 1.0f, 0.2f, 1.0f) :
        glm::vec4(1.0f, 0.2f, 0.2f, 1.0f);

    float boxW = 600;
    float boxH = 450;
    float boxX = width_ / 2 - boxW / 2;
    float boxY = height_ / 2 - boxH / 2;

    uiRenderer_->RenderRect(boxX, boxY, boxW, boxH, mainColor);
    uiRenderer_->RenderBorder(boxX, boxY, boxW, boxH, 6, accentColor);

    float titleH = 100;
    uiRenderer_->RenderRect(boxX + 50, boxY + 30, boxW - 100, titleH, accentColor);
    uiRenderer_->RenderBorder(boxX + 50, boxY + 30, boxW - 100, titleH, 4,
        { 1.0f, 1.0f, 1.0f, 1.0f });

    float charSize = 80;
    float charX = boxX + boxW / 2 - charSize / 2;
    float charY = boxY + 150;

    glm::vec4 charColor = won ?
        glm::vec4(0.92f, 0.92f, 1.0f, 1.0f) :
        glm::vec4(1.0f, 0.63f, 0.35f, 1.0f);

    uiRenderer_->RenderRect(charX, charY, charSize, charSize, charColor);
    uiRenderer_->RenderBorder(charX, charY, charSize, charSize, 3,
        { 1.0f, 1.0f, 1.0f, 1.0f });

    float statsY = charY + 100;
    float statsH = 120;
    uiRenderer_->RenderRect(boxX + 80, statsY, boxW - 160, statsH,
        { 0.1f, 0.1f, 0.1f, 0.9f });
    uiRenderer_->RenderBorder(boxX + 80, statsY, boxW - 160, statsH, 3,
        { 1.0f, 1.0f, 1.0f, 1.0f });

    float scoreBarW = 300;
    float scoreBarH = 40;
    float scoreX = boxX + boxW / 2 - scoreBarW / 2;
    uiRenderer_->RenderRect(scoreX, statsY + 20, scoreBarW, scoreBarH,
        { 1.0f, 0.84f, 0.0f, 0.9f });
    uiRenderer_->RenderBorder(scoreX, statsY + 20, scoreBarW, scoreBarH, 2,
        { 1.0f, 1.0f, 1.0f, 1.0f });

    uiRenderer_->RenderRect(scoreX, statsY + 70, scoreBarW / 2 - 10, 30,
        { 0.3f, 0.5f, 1.0f, 0.9f });

    uiRenderer_->RenderRect(scoreX + scoreBarW / 2 + 10, statsY + 70, scoreBarW / 2 - 10, 30,
        { 1.0f, 0.95f, 0.2f, 0.9f });

    float pulse = (std::sin(gameTime_ * 3.0f) + 1.0f) * 0.5f;
    float promptW = 400;
    float promptH = 60;
    float promptX = width_ / 2 - promptW / 2;
    float promptY = boxY + boxH + 30;

    uiRenderer_->RenderRect(promptX, promptY, promptW, promptH,
        { pulse * 0.5f, pulse * 0.5f, pulse * 0.8f, 0.9f });
    uiRenderer_->RenderBorder(promptX, promptY, promptW, promptH, 3,
        { 1.0f, 1.0f, 1.0f, pulse });

    uiRenderer_->EndUI();

    static bool oncePrinted = false;
    if (!oncePrinted) {
        std::cout << "\n";
        std::cout << "==============================================\n";
        if (won) {
            std::cout << "         JERRY WINS!\n";
            std::cout << "   You collected all the cheese!\n";
        }
        else {
            std::cout << "         TOM WINS!\n";
            std::cout << "      Jerry got caught!\n";
        }
        std::cout << "==============================================\n";
        std::cout << " Final Score: " << score_ << "\n";
        std::cout << " Level Reached: " << level_ << "\n";
        std::cout << " Cheese Collected: " << collected_ << "/" << totalCheese_ << "\n";
        std::cout << "==============================================\n";
        std::cout << " Press R to Restart\n";
        std::cout << " Press ESC to Quit\n";
        std::cout << "==============================================\n\n";
        oncePrinted = true;
    }
}