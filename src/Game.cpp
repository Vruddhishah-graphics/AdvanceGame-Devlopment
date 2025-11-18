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

using namespace game;

// Static members
bool Game::keys_[1024] = { false };

void Game::keyCb(GLFWwindow*, int key, int, int action, int) {
    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)   keys_[key] = true;
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

void Game::spawnParticles(const glm::vec3& pos, const glm::vec3& color, int count) {
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.pos = pos;

        float angle = (float)(std::rand() % 360) * 3.14159265f / 180.f;
        float speed = 2.f + (float)(std::rand() % 100) / 50.f;

        p.vel = glm::vec3(std::cos(angle) * speed,
            3.f + (float)(std::rand() % 100) / 50.f,
            std::sin(angle) * speed);

        p.color = color;
        p.life = 1.0f;
        p.size = 0.1f + (float)(std::rand() % 100) / 500.f;

        particles_.push_back(p);
    }
}

void Game::drawText(float x, float y, const std::string& txt, float r, float g, float b) {
    // Stub
}

void Game::printInstructions() {
    std::cout << "\n";
    std::cout << "================================================================\n";
    std::cout << "       CHEESE CHASE 3D - TOM & JERRY with CANNON!              \n";
    std::cout << "================================================================\n";
    std::cout << " OBJECTIVE:                                                    \n";
    std::cout << "   Jerry must collect ALL cheese before Tom catches him!       \n";
    std::cout << "                                                               \n";
    std::cout << " CONTROLS:                                                     \n";
    std::cout << "   Jerry (Mouse):  W/A/S/D - Move around                      \n";
    std::cout << "   Tom (Cat):      Arrow Keys - Chase Jerry                   \n";
    std::cout << "   Camera:         Q/E - Rotate | Z/X - Height                \n";
    std::cout << "   R - Reset | ESC - Quit                                     \n";
    std::cout << "                                                               \n";
    std::cout << " POWER-UPS:                                                    \n";
    std::cout << "   Gold Sphere  - SHIELD (Invincible 5 sec)                  \n";
    std::cout << "   Cyan Cone    - SPEED BOOST (Faster 5 sec)                 \n";
    std::cout << "   Blue Sphere  - FREEZE TOM (3 seconds)                     \n";
    std::cout << "                                                               \n";
    std::cout << " FEATURES:                                                     \n";
    std::cout << "   * Hierarchical scene graph with animated cannon            \n";
    std::cout << "   * Procedural textures (grass, stone, metal, wood)          \n";
    std::cout << "   * Dynamic lighting system                                  \n";
    std::cout << "   * Multi-level progression                                  \n";
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

    win_ = glfwCreateWindow(width_, height_, "Cheese Chase 3D - Scene Graph Edition", nullptr, nullptr);
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

    glm::vec3 L1pos(8.f, 11.f, 6.f), L1col(1.0f, 1.0f, 0.95f);
    glm::vec3 L2pos(-8.f, 9.f, -6.f), L2col(0.7f, 0.85f, 1.0f);

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
    std::cout << "Models loaded!\n";
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
    std::cout << "Building scene graph with animated cannon...\n";

    sceneRoot_ = std::make_shared<Node>("SceneRoot");

    // ==================== DECORATIVE CANNON IN CORNER ====================
    // Position cannon INSIDE the room, integrated into the scene
    cannonRoot_ = std::make_shared<Node>("CannonRoot");
    cannonRoot_->SetPosition({ -6.5f, 0.0f, -4.0f }); // Back-left corner, inside play area
    sceneRoot_->AddChild(cannonRoot_);

    // Cannon base (stone pedestal) - dark gray stone
    auto cannonBase = std::make_shared<Node>("CannonBase");
    cannonBase->SetPosition({ 0.0f, 0.8f, 0.0f });
    cannonBase->SetScale({ 0.8f, 1.6f, 0.8f });
    cannonBase->SetRenderCallback([this](const glm::mat4& transform) {
        glUseProgram(prog_);
        glUniformMatrix4fv(uModel_, 1, GL_FALSE, glm::value_ptr(transform));
        glUniform1i(uUseTexture_, 1);
        glUniform1f(uKa_, 0.4f);
        glUniform1f(uKd_, 0.6f);
        glUniform1f(uKs_, 0.2f);
        glUniform1f(uShine_, 32.0f);
        glUniform1f(uEmissive_, 0.0f);
        stoneTex_->Bind(0);
        drawMesh(cyl_);
        });
    cannonRoot_->AddChild(cannonBase);

    // Cannon turret (rotating platform) - metallic
    auto turret = std::make_shared<Node>("Turret");
    turret->SetPosition({ 0.0f, 1.6f, 0.0f });
    turret->SetScale({ 1.0f, 0.3f, 1.0f });
    turret->SetRenderCallback([this](const glm::mat4& transform) {
        glUseProgram(prog_);
        glUniformMatrix4fv(uModel_, 1, GL_FALSE, glm::value_ptr(transform));
        glUniform1i(uUseTexture_, 1);
        glUniform1f(uKa_, 0.3f);
        glUniform1f(uKd_, 0.6f);
        glUniform1f(uKs_, 0.6f);
        glUniform1f(uShine_, 64.0f);
        glUniform1f(uEmissive_, 0.0f);
        metalTex_->Bind(0);
        drawMesh(cyl_);
        });
    cannonRoot_->AddChild(turret);

    // Cannon barrel hinge (pivot point for tilting)
    cannonBarrel_ = std::make_shared<Node>("BarrelHinge");
    cannonBarrel_->SetPosition({ 0.0f, 1.6f, 0.0f });
    cannonRoot_->AddChild(cannonBarrel_);

    // Cannon barrel (the actual gun barrel) - horizontal metallic cylinder
    auto barrel = std::make_shared<Node>("Barrel");
    barrel->SetPosition({ 1.2f, 0.0f, 0.0f }); // Extended forward
    barrel->SetRotation(90.0f, { 0, 0, 1 }); // Rotate to point horizontally
    barrel->SetScale({ 0.15f, 1.8f, 0.15f }); // Long thin barrel
    barrel->SetRenderCallback([this](const glm::mat4& transform) {
        glUseProgram(prog_);
        glUniformMatrix4fv(uModel_, 1, GL_FALSE, glm::value_ptr(transform));
        glUniform1i(uUseTexture_, 1);
        glUniform1f(uKa_, 0.2f);
        glUniform1f(uKd_, 0.7f);
        glUniform1f(uKs_, 0.8f);
        glUniform1f(uShine_, 128.0f);
        glUniform1f(uEmissive_, 0.0f);
        metalTex_->Bind(0);
        drawMesh(cyl_);
        });
    cannonBarrel_->AddChild(barrel);

    // Cannon muzzle cap (decorative sphere at end)
    auto muzzle = std::make_shared<Node>("Muzzle");
    muzzle->SetPosition({ 2.1f, 0.0f, 0.0f });
    muzzle->SetScale({ 0.2f, 0.2f, 0.2f });
    muzzle->SetRenderCallback([this](const glm::mat4& transform) {
        glUseProgram(prog_);
        glUniformMatrix4fv(uModel_, 1, GL_FALSE, glm::value_ptr(transform));
        glUniform1i(uUseTexture_, 0);
        glUniform3f(uBaseColor_, 0.2f, 0.2f, 0.2f);
        glUniform1f(uKa_, 0.3f);
        glUniform1f(uKd_, 0.5f);
        glUniform1f(uKs_, 0.9f);
        glUniform1f(uShine_, 128.0f);
        glUniform1f(uEmissive_, 0.0f);
        drawMesh(sphere_);
        });
    cannonBarrel_->AddChild(muzzle);

    std::cout << "Scene graph created! Cannon positioned at (-6.5, 0, -4)\n";
}

void Game::resetWorld() {
    cam_.setProjection(45.f, float(width_) / float(height_), 0.1f, 100.f);

    // Initial camera position
    cameraAngle_ = 0.0f;
    cameraHeight_ = 18.0f;
    cameraDistance_ = 20.0f;

    // Players
    mouse_ = {};
    mouse_.pos = { -4.f, 0.4f, -2.f };
    mouse_.size = { 0.9f, 0.9f, 0.9f };
    mouse_.color = { 0.92f, 0.92f, 1.0f };
    mouse_.speed = 4.2f;

    cat_ = {};
    cat_.pos = { 3.5f, 0.4f, 2.0f };
    cat_.size = { 1.0f, 1.2f, 1.0f };
    cat_.color = { 1.0f, 0.63f, 0.35f };
    cat_.speed = 3.5f + level_ * 0.25f;

    // Walls
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

    // Furniture
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
    addF({ 0.0f, 0.6f,  0.0f }, { 3.0f, 1.2f, 1.0f }, { 0.86f, 0.57f, 0.40f });
    addF({ 2.0f, 0.5f,  2.5f }, { 1.7f, 1.0f, 1.5f }, { 0.45f, 0.64f, 0.86f });

    // Cheese
    cheeses_.clear();
    int cheeseCount = 4 + level_;
    for (int i = 0; i < cheeseCount; ++i) {
        float x = -7.f + (rand() % 140) / 10.0f;
        float z = -5.f + (rand() % 100) / 10.0f;
        Cheese c;
        c.pos = glm::vec3(x, 0.35f, z);
        c.taken = false;
        cheeses_.push_back(c);
    }

    // Power-ups
    powerups_.clear();
    for (int i = 0; i < 3; ++i) {
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
    gameOver_ = false;
    mouseWin_ = false;

    mouseInvincible_ = false;
    mouseSpeedBoost_ = false;
    catFrozen_ = false;
    powerUpTimer_ = 0.f;
    currentPowerUp_ = -1;
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

bool Game::intersects(const AABB& a, const AABB& b) {
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

    float mx = std::min(x1, x2);
    float my = std::min(y1, y2);
    float mz = std::min(z1, z2);

    if (mx < my && mx < mz) return glm::vec3((x1 < x2 ? -x1 : x2), 0, 0);
    if (my < mz)            return glm::vec3(0, (y1 < y2 ? -y1 : y2), 0);
    return glm::vec3(0, 0, (z1 < z2 ? -z1 : z2));
}

void Game::updateCannon(float dt) {
    // Slowly rotate cannon base (full 360 degree rotation)
    cannonRotation_ += 20.0f * dt;
    if (cannonRotation_ > 360.0f) cannonRotation_ -= 360.0f;
    cannonRoot_->SetRotation(cannonRotation_, { 0, 1, 0 });

    // Tilt barrel up and down smoothly
    static float tiltDir = 1.0f;
    cannonTilt_ += 12.0f * dt * tiltDir;
    if (cannonTilt_ > 25.0f) { cannonTilt_ = 25.0f; tiltDir = -1.0f; }
    if (cannonTilt_ < -15.0f) { cannonTilt_ = -15.0f; tiltDir = 1.0f; }
    cannonBarrel_->SetRotation(cannonTilt_, { 0, 0, 1 });
}

void Game::update(float dt) {
    if (keys_[GLFW_KEY_ESCAPE])
        glfwSetWindowShouldClose(win_, GL_TRUE);

    if (keys_[GLFW_KEY_R]) {
        level_ = 1;
        resetWorld();
    }

    // Camera controls
    if (keys_[GLFW_KEY_Q]) cameraAngle_ -= 1.5f * dt;
    if (keys_[GLFW_KEY_E]) cameraAngle_ += 1.5f * dt;
    if (keys_[GLFW_KEY_Z]) cameraHeight_ = std::max(5.0f, cameraHeight_ - 8.0f * dt);
    if (keys_[GLFW_KEY_X]) cameraHeight_ = std::min(30.0f, cameraHeight_ + 8.0f * dt);

    // Update camera position
    glm::vec3 camPos(
        cameraDistance_ * std::cos(cameraAngle_),
        cameraHeight_,
        cameraDistance_ * std::sin(cameraAngle_)
    );
    cam_.setPosition(camPos);
    cam_.setTarget(glm::vec3(0.f, 0.f, 0.f));

    // Update cannon animation
    updateCannon(dt);

    // Update scene graph
    if (sceneRoot_) {
        sceneRoot_->Update(dt);
    }

    // Particles
    for (auto it = particles_.begin(); it != particles_.end();) {
        it->pos += it->vel * dt;
        it->vel.y -= 9.8f * dt;
        it->life -= dt;

        if (it->life <= 0.f) it = particles_.erase(it);
        else ++it;
    }

    // Power-up timer
    if (powerUpTimer_ > 0.f) {
        powerUpTimer_ -= dt;
        if (powerUpTimer_ <= 0.f) {
            mouseInvincible_ = false;
            mouseSpeedBoost_ = false;
            catFrozen_ = false;
            currentPowerUp_ = -1;
        }
    }

    // Mouse movement (WASD) - W=forward, S=back, A=left, D=right
    glm::vec3 mv(0);
    if (keys_[GLFW_KEY_W]) mv.z -= 1;  // Forward (negative Z)
    if (keys_[GLFW_KEY_S]) mv.z += 1;  // Backward (positive Z)
    if (keys_[GLFW_KEY_A]) mv.x -= 1;  // Left (negative X)
    if (keys_[GLFW_KEY_D]) mv.x += 1;  // Right (positive X)
    if (glm::length(mv) > 0.f) mv = glm::normalize(mv);

    float currentSpeed = mouse_.speed;
    if (mouseSpeedBoost_) currentSpeed *= 1.6f;
    mouse_.pos += mv * currentSpeed * dt;

    // Cat movement (Arrow Keys) - FIXED CONTROLS
    // UP=forward, DOWN=back, LEFT=left, RIGHT=right
    if (!catFrozen_) {
        glm::vec3 cv(0);
        if (keys_[GLFW_KEY_UP])    cv.z -= 1;  // Forward (negative Z)
        if (keys_[GLFW_KEY_DOWN])  cv.z += 1;  // Backward (positive Z)
        if (keys_[GLFW_KEY_LEFT])  cv.x -= 1;  // Left (negative X)
        if (keys_[GLFW_KEY_RIGHT]) cv.x += 1;  // Right (positive X)

        if (glm::length(cv) > 0.f) {
            cv = glm::normalize(cv);
            cat_.pos += cv * cat_.speed * dt;
        }
    }

    // Collision resolution
    auto resolveWith = [&](std::vector<Entity>& blocks, Player& p) {
        for (const auto& w : blocks) {
            AABB a = p.bounds();
            AABB b = w.bounds();
            if (intersects(a, b)) {
                p.pos += overlapVec(a, b);
            }
        }
        };

    resolveWith(walls_, mouse_);
    resolveWith(walls_, cat_);

    // Furniture pushing
    for (auto& f : furniture_) {
        {
            AABB a = mouse_.bounds();
            AABB b = f.bounds();
            if (intersects(a, b)) {
                glm::vec3 o = overlapVec(a, b);
                if (f.dynamic) f.pos -= 0.7f * o;
                mouse_.pos += 0.3f * o;
            }
        }
        {
            AABB a = cat_.bounds();
            AABB b = f.bounds();
            if (intersects(a, b)) {
                glm::vec3 o = overlapVec(a, b);
                if (f.dynamic) f.pos -= 0.7f * o;
                cat_.pos += 0.3f * o;
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

    // Cheese collection
    for (auto& c : cheeses_) {
        if (c.taken) continue;
        AABB a = mouse_.bounds();
        AABB b{ c.pos - glm::vec3(0.25f), c.pos + glm::vec3(0.25f) };
        if (intersects(a, b)) {
            c.taken = true;
            ++collected_;
            spawnParticles(c.pos, glm::vec3(1.0f, 0.95f, 0.2f), 20);
        }
    }

    // Power-up collection
    for (auto& p : powerups_) {
        if (p.taken) continue;

        p.rotation += dt * 2.0f;

        AABB a = mouse_.bounds();
        AABB b{ p.pos - glm::vec3(0.3f), p.pos + glm::vec3(0.3f) };
        if (intersects(a, b)) {
            p.taken = true;
            currentPowerUp_ = p.type;
            powerUpTimer_ = 5.0f;

            if (p.type == 0) {
                mouseInvincible_ = true;
                spawnParticles(p.pos, glm::vec3(1.0f, 0.84f, 0.0f), 30);
            }
            else if (p.type == 1) {
                mouseSpeedBoost_ = true;
                spawnParticles(p.pos, glm::vec3(0.0f, 1.0f, 1.0f), 30);
            }
            else {
                catFrozen_ = true;
                powerUpTimer_ = 3.0f;
                spawnParticles(p.pos, glm::vec3(0.2f, 0.4f, 1.0f), 30);
            }
        }
    }

    // Catch detection
    if (glm::length(mouse_.pos - cat_.pos) < 0.8f) {
        if (!mouseInvincible_) {
            gameOver_ = true;
            mouseWin_ = false;
            spawnParticles(mouse_.pos, glm::vec3(1.0f, 0.0f, 0.0f), 50);
        }
    }

    // Win condition
    if (collected_ == (int)cheeses_.size()) {
        gameOver_ = true;
        mouseWin_ = true;
        ++level_;
        spawnParticles(mouse_.pos, glm::vec3(0.0f, 1.0f, 0.0f), 50);
    }

    // Window title
    std::string title = "Cheese Chase 3D | Camera: Q/E/Z/X | Mouse:WASD Cat:Arrows | Level:" + std::to_string(level_) +
        " | Cheese:" + std::to_string(collected_) + "/" + std::to_string((int)cheeses_.size());

    if (currentPowerUp_ >= 0) {
        title += " | PowerUp:";
        if (currentPowerUp_ == 0) title += "SHIELD";
        else if (currentPowerUp_ == 1) title += "SPEED";
        else title += "FREEZE";
        title += "(" + std::to_string((int)std::ceil(std::max(powerUpTimer_, 0.f))) + "s)";
    }

    if (gameOver_) {
        if (mouseWin_) title += " | MOUSE WINS! Press R";
        else title += " | CAT WINS! Press R";
    }

    glfwSetWindowTitle(win_, title.c_str());
}

static void setMaterial(GLint uBaseColor, GLint uEmissive, GLint uKa, GLint uKd,
    GLint uKs, GLint uShine,
    const glm::vec3& color, float emissive)
{
    glUniform3fv(uBaseColor, 1, glm::value_ptr(color));
    glUniform1f(uEmissive, emissive);
    glUniform1f(uKa, 0.45f);
    glUniform1f(uKd, 0.75f);
    glUniform1f(uKs, 0.18f);
    glUniform1f(uShine, 24.0f);
}

void Game::render() {
    int W, H;
    glfwGetFramebufferSize(win_, &W, &H);
    glViewport(0, 0, W, H);

    glClearColor(0.60f, 0.86f, 1.00f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(prog_);

    glm::mat4 V = cam_.view();
    glm::mat4 P = cam_.proj();

    glUniformMatrix4fv(uView_, 1, GL_FALSE, glm::value_ptr(V));
    glUniformMatrix4fv(uProj_, 1, GL_FALSE, glm::value_ptr(P));
    glUniform3fv(uViewPos_, 1, glm::value_ptr(cam_.position()));
    glUniform1i(uTexture_, 0);

    // Render scene graph (includes animated cannon)
    if (sceneRoot_) {
        sceneRoot_->Render();
    }

    // Floor with grass texture
    {
        glm::mat4 M(1.f);
        M = glm::translate(M, { 0.f, -0.01f, 0.f });
        M = glm::scale(M, { 18.f, 0.02f, 12.f });
        glUniformMatrix4fv(uModel_, 1, GL_FALSE, glm::value_ptr(M));

        glUniform1i(uUseTexture_, 1);
        glUniform1f(uKa_, 0.5f);
        glUniform1f(uKd_, 0.7f);
        glUniform1f(uKs_, 0.1f);
        glUniform1f(uShine_, 16.0f);
        glUniform1f(uEmissive_, 0.0f);
        grassTex_->Bind(0);
        drawMesh(box_);
    }

    // Walls with stone texture
    glUniform1i(uUseTexture_, 1);
    for (const auto& w : walls_) {
        glm::mat4 M(1.f);
        M = glm::translate(M, w.pos);
        M = glm::scale(M, w.size);
        glUniformMatrix4fv(uModel_, 1, GL_FALSE, glm::value_ptr(M));

        glUniform1f(uKa_, 0.4f);
        glUniform1f(uKd_, 0.6f);
        glUniform1f(uKs_, 0.2f);
        glUniform1f(uShine_, 32.0f);
        glUniform1f(uEmissive_, 0.0f);
        stoneTex_->Bind(0);
        drawMesh(box_);
    }

    // Furniture with wood texture
    for (const auto& f : furniture_) {
        glm::mat4 M(1.f);
        M = glm::translate(M, f.pos);
        M = glm::scale(M, f.size);
        glUniformMatrix4fv(uModel_, 1, GL_FALSE, glm::value_ptr(M));

        glUniform1i(uUseTexture_, 1);
        glUniform1f(uKa_, 0.3f);
        glUniform1f(uKd_, 0.7f);
        glUniform1f(uKs_, 0.2f);
        glUniform1f(uShine_, 16.0f);
        glUniform1f(uEmissive_, 0.0f);
        woodTex_->Bind(0);
        drawMesh(box_);
    }

    // Disable texture for game objects
    glUniform1i(uUseTexture_, 0);

    // Cheese pieces
    for (const auto& c : cheeses_) {
        if (c.taken) continue;

        glm::mat4 M(1.f);
        M = glm::translate(M, c.pos);
        M = glm::scale(M, { 0.45f, 0.45f, 0.45f });
        glUniformMatrix4fv(uModel_, 1, GL_FALSE, glm::value_ptr(M));

        setMaterial(uBaseColor_, uEmissive_, uKa_, uKd_, uKs_, uShine_,
            { 1.0f, 0.95f, 0.2f }, 0.25f);

        drawMesh(cheeseModel_);
    }

    // Power-ups with rotation
    glDisable(GL_CULL_FACE);
    for (const auto& p : powerups_) {
        if (p.taken) continue;

        glm::mat4 M(1.f);
        M = glm::translate(M, p.pos);
        M = glm::rotate(M, p.rotation, glm::vec3(0.f, 1.f, 0.f));
        M = glm::scale(M, { 0.35f, 0.35f, 0.35f });
        glUniformMatrix4fv(uModel_, 1, GL_FALSE, glm::value_ptr(M));

        if (p.type == 0) {
            setMaterial(uBaseColor_, uEmissive_, uKa_, uKd_, uKs_, uShine_,
                { 1.0f, 0.84f, 0.0f }, 0.8f);
            drawMesh(sphere_);
        }
        else if (p.type == 1) {
            setMaterial(uBaseColor_, uEmissive_, uKa_, uKd_, uKs_, uShine_,
                { 0.0f, 1.0f, 1.0f }, 0.9f);
            drawMesh(cone_);
        }
        else {
            setMaterial(uBaseColor_, uEmissive_, uKa_, uKd_, uKs_, uShine_,
                { 0.3f, 0.5f, 1.0f }, 0.9f);
            drawMesh(sphere_);
        }
    }
    glEnable(GL_CULL_FACE);

    // Particles
    glDisable(GL_CULL_FACE);
    for (const auto& p : particles_) {
        glm::mat4 M(1.f);
        M = glm::translate(M, p.pos);
        M = glm::scale(M, { p.size, p.size, p.size });
        glUniformMatrix4fv(uModel_, 1, GL_FALSE, glm::value_ptr(M));

        setMaterial(uBaseColor_, uEmissive_, uKa_, uKd_, uKs_, uShine_,
            p.color, p.life);

        drawMesh(sphere_);
    }
    glEnable(GL_CULL_FACE);

    // Jerry (Mouse)
    {
        glm::mat4 M(1.f);
        M = glm::translate(M, mouse_.pos);
        M = glm::scale(M, mouse_.size * glm::vec3(0.9f));
        glUniformMatrix4fv(uModel_, 1, GL_FALSE, glm::value_ptr(M));

        setMaterial(uBaseColor_, uEmissive_, uKa_, uKd_, uKs_, uShine_,
            mouse_.color, 0.05f);

        drawMesh(mouseModel_);
    }

    // Tom (Cat)
    {
        glm::mat4 M(1.f);
        M = glm::translate(M, cat_.pos);
        M = glm::scale(M, cat_.size);
        glUniformMatrix4fv(uModel_, 1, GL_FALSE, glm::value_ptr(M));

        setMaterial(uBaseColor_, uEmissive_, uKa_, uKd_, uKs_, uShine_,
            cat_.color, 0.05f);

        drawMesh(catModel_);
    }
}