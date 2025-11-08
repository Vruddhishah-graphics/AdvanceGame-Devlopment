#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>

#include "game/Camera.h"
#include "game/MeshUtils.h"

namespace game {

struct AABB { glm::vec3 min, max; };

struct Entity {
    glm::vec3 pos{0.f}, size{1.f}, color{1.f};
    bool dynamic = false;
    AABB bounds() const { glm::vec3 r = size * 0.5f; return {pos - r, pos + r}; }
};

struct Player {
    glm::vec3 pos{0.f}, color{1.f}; glm::vec3 size{1.f};
    float speed = 4.0f;
    AABB bounds() const { glm::vec3 r = size * 0.5f; return {pos - r, pos + r}; }
};

struct Cheese { glm::vec3 pos; bool taken = false; };

class Game {
public:
    void Run();

private:
    void initWindow();
    void initGL();
    void initShaders();
    void initMeshes();
    void resetWorld();
    void loop();
    void update(float dt);
    void render();

    static bool keys_[1024];
    static void keyCb(GLFWwindow*, int key, int, int action, int);

    GLuint compile(GLenum type, const std::string& src);
    GLuint link(GLuint v, GLuint f);
    std::string loadText(const std::string& path);

    static bool intersects(const AABB& a, const AABB& b);
    static glm::vec3 overlapVec(const AABB& a, const AABB& b);

private:
    GLFWwindow* win_ = nullptr;
    int width_ = 1280, height_ = 720;

    Camera cam_;

    GLuint prog_ = 0;
    GLint uModel_=-1, uView_=-1, uProj_=-1, uViewPos_=-1, uBaseColor_=-1, uEmissive_=-1, uKa_=-1, uKd_=-1, uKs_=-1, uShine_=-1;
    GLint uL1pos_=-1, uL1col_=-1, uL2pos_=-1, uL2col_=-1;

    Mesh box_{}, sphere_{}, cone_{}, cyl_{};

    Player mouse_{}, cat_{};
    std::vector<Entity> walls_;
    std::vector<Entity> furniture_;
    std::vector<Cheese> cheeses_;
    int collected_ = 0;
    bool gameOver_ = false, mouseWin_ = false;
};

} // namespace game
