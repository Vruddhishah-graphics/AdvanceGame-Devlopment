#include "game/Game.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace game;

bool Game::keys_[1024] = {false};
void Game::keyCb(GLFWwindow*, int key, int, int action, int) {
    if (key>=0 && key<1024) {
        if (action==GLFW_PRESS) keys_[key]=true;
        if (action==GLFW_RELEASE) keys_[key]=false;
    }
}

std::string Game::loadText(const std::string& p) {
    std::ifstream f(p); std::stringstream ss; ss << f.rdbuf(); return ss.str();
}
GLuint Game::compile(GLenum type, const std::string& src) {
    GLuint s = glCreateShader(type); const char* c = src.c_str(); glShaderSource(s,1,&c,nullptr); glCompileShader(s);
    GLint ok=0; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) { GLint n=0; glGetShaderiv(s,GL_INFO_LOG_LENGTH,&n); std::string log(n,'\0'); glGetShaderInfoLog(s,n,nullptr,log.data()); fprintf(stderr,"Shader error:\n%s\n",log.c_str()); std::exit(1); }
    return s;
}
GLuint Game::link(GLuint v, GLuint f) {
    GLuint p = glCreateProgram(); glAttachShader(p,v); glAttachShader(p,f); glLinkProgram(p);
    GLint ok=0; glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) { GLint n=0; glGetProgramiv(p,GL_INFO_LOG_LENGTH,&n); std::string log(n,'\0'); glGetProgramInfoLog(p,n,nullptr,log.data()); fprintf(stderr,"Link error:\n%s\n",log.c_str()); std::exit(1); }
    glDeleteShader(v); glDeleteShader(f); return p;
}

void Game::Run() {
    initWindow();
    initGL();
    initShaders();
    initMeshes();
    resetWorld();
    loop();
    glfwDestroyWindow(win_);
    glfwTerminate();
}

void Game::initWindow() {
    if (!glfwInit()) { fprintf(stderr,"GLFW init failed\n"); std::exit(1); }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
#if defined(__APPLE__)
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    win_ = glfwCreateWindow(width_, height_, "Cheese Chase 3D – Neutral Studio Lighting", nullptr, nullptr);
    if (!win_) { fprintf(stderr,"Window create failed\n"); std::exit(1); }
    glfwMakeContextCurrent(win_); glfwSwapInterval(1);
    glfwSetKeyCallback(win_, keyCb);
}

void Game::initGL() {
    glewExperimental = GL_TRUE;
    GLenum ge = glewInit(); glGetError();
    if (ge != GLEW_OK) { fprintf(stderr,"GLEW init failed\n"); std::exit(1); }
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
}

void Game::initShaders() {
    std::string base = ASSET_DIR;
    std::string vsPath = base + std::string("/shaders/basic.vert");
    std::string fsPath = base + std::string("/shaders/basic.frag");
    std::string vsSrc = loadText(vsPath);
    std::string fsSrc = loadText(fsPath);
    GLuint v = compile(GL_VERTEX_SHADER, vsSrc);
    GLuint f = compile(GL_FRAGMENT_SHADER, fsSrc);
    prog_ = link(v,f);

    glUseProgram(prog_);
    uModel_ = glGetUniformLocation(prog_, "uModel");
    uView_  = glGetUniformLocation(prog_, "uView");
    uProj_  = glGetUniformLocation(prog_, "uProj");
    uViewPos_= glGetUniformLocation(prog_, "uViewPos");
    uBaseColor_= glGetUniformLocation(prog_, "uBaseColor");
    uEmissive_= glGetUniformLocation(prog_, "uEmissive");
    uKa_ = glGetUniformLocation(prog_, "uKa");
    uKd_ = glGetUniformLocation(prog_, "uKd");
    uKs_ = glGetUniformLocation(prog_, "uKs");
    uShine_ = glGetUniformLocation(prog_, "uShine");
    uL1pos_ = glGetUniformLocation(prog_, "uL1.position");
    uL1col_ = glGetUniformLocation(prog_, "uL1.color");
    uL2pos_ = glGetUniformLocation(prog_, "uL2.position");
    uL2col_ = glGetUniformLocation(prog_, "uL2.color");

    glm::vec3 L1pos( 6.f, 7.f,  6.f), L1col(0.95f, 0.95f, 0.95f);
    glm::vec3 L2pos(-6.f, 7.f, -6.f), L2col(0.95f, 0.95f, 0.95f);
    glUniform3fv(uL1pos_, 1, glm::value_ptr(L1pos)); glUniform3fv(uL1col_, 1, glm::value_ptr(L1col));
    glUniform3fv(uL2pos_, 1, glm::value_ptr(L2pos)); glUniform3fv(uL2col_, 1, glm::value_ptr(L2col));
}

void Game::initMeshes() {
    box_ = makeBox();
    sphere_ = makeSphere();
    cyl_ = makeCylinder();
    cone_ = makeCone();
}

void Game::resetWorld() {
    cam_.setProjection(50.f, float(width_)/height_, 0.1f, 100.f);
    cam_.setPosition({0.f,6.f,12.f});
    cam_.setTarget({0.f,0.f,0.f});

    mouse_ = {}; mouse_.pos={-3.f,0.4f,-2.f}; mouse_.size={0.8f,0.8f,0.8f}; mouse_.color={0.8f,0.8f,0.9f}; mouse_.speed=4.2f;
    cat_   = {}; cat_.pos={ 3.f,0.4f, 2.f};   cat_.size={0.9f,1.2f,0.9f};   cat_.color={1.0f,0.6f,0.3f};   cat_.speed=4.5f;

    walls_.clear();
    auto wall = [&](float x,float z,float sx,float sz){
        Entity w; w.pos={x,1.f,z}; w.size={sx,2.f,sz}; w.color={0.90f,0.90f,0.94f}; walls_.push_back(w);
    };
    const float W=18.f, D=12.f;
    wall(0.f,-D*0.5f,W,1.f); wall(0.f,D*0.5f,W,1.f);
    wall(-W*0.5f,0.f,1.f,D); wall(W*0.5f,0.f,1.f,D);

    furniture_.clear();
    auto addF = [&](glm::vec3 p, glm::vec3 s, glm::vec3 c){
        Entity f; f.pos=p; f.size=s; f.color=c; f.dynamic=true; furniture_.push_back(f);
    };
    addF({-4.0f,0.5f,-1.5f},{2.0f,1.0f,1.2f},{0.55f,0.45f,0.35f});
    addF({ 2.0f,0.5f, 2.5f},{1.5f,1.0f,1.5f},{0.35f,0.50f,0.65f});
    addF({ 0.0f,0.6f, 0.0f},{3.0f,1.2f,1.0f},{0.50f,0.38f,0.28f});

    cheeses_.clear();
    for (auto p : { glm::vec3(-5.f,0.35f, 3.f), glm::vec3(-1.f,0.35f,-3.f), glm::vec3(4.5f,0.35f,2.5f), glm::vec3(0.f,0.35f,4.f)})
        cheeses_.push_back({p,false});

    collected_=0; gameOver_=false; mouseWin_=false;
}

void Game::loop() {
    double prev = glfwGetTime();
    while (!glfwWindowShouldClose(win_)) {
        double now = glfwGetTime(); float dt=float(now - prev); prev=now;
        update(dt); render();
        glfwSwapBuffers(win_); glfwPollEvents();
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
    if (x1<=0.f || x2<=0.f || y1<=0.f || y2<=0.f || z1<=0.f || z2<=0.f) return glm::vec3(0);
    float mx = std::min(x1,x2), my=std::min(y1,y2), mz=std::min(z1,z2);
    if (mx<my && mx<mz) return glm::vec3((x1<x2?-x1:x2), 0, 0);
    if (my<mz)         return glm::vec3(0, (y1<y2?-y1:y2), 0);
    return glm::vec3(0,0,(z1<z2?-z1:z2));
}

void Game::update(float dt) {
    if (keys_[GLFW_KEY_ESCAPE]) glfwSetWindowShouldClose(win_, GL_TRUE);
    if (keys_[GLFW_KEY_R]) resetWorld();

    glm::vec3 mv(0);
    if (keys_[GLFW_KEY_W]) mv.z -= 1;
    if (keys_[GLFW_KEY_S]) mv.z += 1;
    if (keys_[GLFW_KEY_A]) mv.x -= 1;
    if (keys_[GLFW_KEY_D]) mv.x += 1;
    if (glm::length(mv)>0) mv=glm::normalize(mv);
    mouse_.pos += mv * mouse_.speed * dt;

    glm::vec3 cv(0);
    if (keys_[GLFW_KEY_UP])    cv.z -= 1;
    if (keys_[GLFW_KEY_DOWN])  cv.z += 1;
    if (keys_[GLFW_KEY_LEFT])  cv.x -= 1;
    if (keys_[GLFW_KEY_RIGHT]) cv.x += 1;
    if (glm::length(cv)>0) cv=glm::normalize(cv);
    cat_.pos += cv * cat_.speed * dt;

    auto resolveWith = [&](std::vector<Entity>& blocks, Player& p){
        for (const auto& w : blocks) {
            AABB a=p.bounds(), b=w.bounds();
            if (intersects(a,b)) p.pos += overlapVec(a,b);
        }
    };
    resolveWith(walls_, mouse_);
    resolveWith(walls_, cat_);

    for (auto& f : furniture_) {
        AABB a = mouse_.bounds(), b = f.bounds();
        if (intersects(a,b)) { auto o=overlapVec(a,b); if (f.dynamic) f.pos -= 0.7f*o; mouse_.pos += 0.3f*o; }
        a = cat_.bounds(); b = f.bounds();
        if (intersects(a,b)) { auto o=overlapVec(a,b); if (f.dynamic) f.pos -= 0.7f*o; cat_.pos += 0.3f*o; }
        for (const auto& w : walls_) {
            auto fb=f.bounds(), wb=w.bounds();
            if (intersects(fb,wb)) f.pos += overlapVec(fb,wb);
        }
    }

    for (auto& c : cheeses_) {
        if (c.taken) continue;
        AABB a = mouse_.bounds(); AABB b{ c.pos - glm::vec3(0.25f), c.pos + glm::vec3(0.25f)};
        if (intersects(a,b)) { c.taken=true; collected_++; }
    }

    if (glm::length(mouse_.pos - cat_.pos) < 0.8f) { gameOver_=true; mouseWin_=false; }
    if (collected_==(int)cheeses_.size()) { gameOver_=true; mouseWin_=true; }

    std::string t = "Cheese Chase 3D | Mouse=WASD  Cat=Arrows  R=Reset  ESC=Quit | Cheese: " +
        std::to_string(collected_) + "/" + std::to_string((int)cheeses_.size());
    if (gameOver_) t += mouseWin_ ? " | Mouse WINS!" : " | Cat WINS!";
    glfwSetWindowTitle(win_, t.c_str());

    cam_.updateTracking(mouse_.pos, cat_.pos);
}

static void setMaterial(GLint uBaseColor, GLint uEmissive, GLint uKa, GLint uKd, GLint uKs, GLint uShine,
                        const glm::vec3& color, float emissive) {
    glUniform3fv(uBaseColor,1, glm::value_ptr(color));
    glUniform1f(uEmissive, emissive);
    glUniform1f(uKa, 0.25f);
    glUniform1f(uKd, 0.90f);
    glUniform1f(uKs, 0.20f);
    glUniform1f(uShine, 32.f);
}

void Game::render() {
    int W,H; glfwGetFramebufferSize(win_, &W, &H);
    glViewport(0,0,W,H);
    glClearColor(0.16f, 0.16f, 0.18f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(prog_);
    glm::mat4 V = cam_.view();
    glm::mat4 P = cam_.proj();
    glUniformMatrix4fv(uView_,1,GL_FALSE, glm::value_ptr(V));
    glUniformMatrix4fv(uProj_,1,GL_FALSE, glm::value_ptr(P));
    glUniform3fv(uViewPos_,1, glm::value_ptr(cam_.position()));

    { // floor
        glm::mat4 M(1.f); M = glm::translate(M, {0.f,-0.01f,0.f}); M = glm::scale(M, {18.f,0.02f,12.f});
        glUniformMatrix4fv(uModel_,1,GL_FALSE, glm::value_ptr(M));
        setMaterial(uBaseColor_, uEmissive_, uKa_, uKd_, uKs_, uShine_, {0.82f,0.82f,0.82f}, 0.f);
        drawMesh(box_);
    }
    for (const auto& w : walls_) { // walls
        glm::mat4 M(1.f); M = glm::translate(M, w.pos); M = glm::scale(M, w.size);
        glUniformMatrix4fv(uModel_,1,GL_FALSE, glm::value_ptr(M));
        setMaterial(uBaseColor_, uEmissive_, uKa_, uKd_, uKs_, uShine_, {0.90f,0.90f,0.94f}, 0.f);
        drawMesh(box_);
    }
    for (const auto& f : furniture_) { // furniture
        glm::mat4 M(1.f); M = glm::translate(M, f.pos); M = glm::scale(M, f.size);
        glUniformMatrix4fv(uModel_,1,GL_FALSE, glm::value_ptr(M));
        setMaterial(uBaseColor_, uEmissive_, uKa_, uKd_, uKs_, uShine_, f.color, 0.f);
        drawMesh(box_);
    }
    for (const auto& c : cheeses_) { // cheese
        if (c.taken) continue;
        glm::mat4 M(1.f); M = glm::translate(M, c.pos); M = glm::scale(M, {0.35f,0.35f,0.35f});
        glUniformMatrix4fv(uModel_,1,GL_FALSE, glm::value_ptr(M));
        setMaterial(uBaseColor_, uEmissive_, uKa_, uKd_, uKs_, uShine_, {1.0f,0.95f,0.2f}, 0.6f);
        drawMesh(cone_);
    }
    { // mouse
        glm::mat4 M(1.f); M = glm::translate(M, mouse_.pos); M = glm::scale(M, mouse_.size);
        glUniformMatrix4fv(uModel_,1,GL_FALSE, glm::value_ptr(M));
        setMaterial(uBaseColor_, uEmissive_, uKa_, uKd_, uKs_, uShine_, mouse_.color, 0.f);
        drawMesh(sphere_);
        for (float s : {-0.25f,0.25f}) {
            glm::mat4 Me(1.f); Me = glm::translate(Me, mouse_.pos + glm::vec3(s*mouse_.size.x, 0.6f*mouse_.size.y, 0.f)); Me = glm::scale(Me, {0.25f,0.25f,0.25f});
            glUniformMatrix4fv(uModel_,1,GL_FALSE, glm::value_ptr(Me));
            setMaterial(uBaseColor_, uEmissive_, uKa_, uKd_, uKs_, uShine_, mouse_.color, 0.f);
            drawMesh(sphere_);
        }
        glm::mat4 Mt(1.f); Mt = glm::translate(Mt, mouse_.pos + glm::vec3(0.f,-0.1f,0.45f)); Mt = glm::scale(Mt, {0.1f,0.8f,0.1f});
        glUniformMatrix4fv(uModel_,1,GL_FALSE, glm::value_ptr(Mt));
        setMaterial(uBaseColor_, uEmissive_, uKa_, uKd_, uKs_, uShine_, {0.95f,0.6f,0.7f}, 0.f);
        drawMesh(cyl_);
    }
    { // cat
        glm::vec3 p = cat_.pos;
        glm::mat4 Mb(1.f); Mb = glm::translate(Mb, p); Mb = glm::scale(Mb, {0.9f, cat_.size.y, 0.9f});
        glUniformMatrix4fv(uModel_,1,GL_FALSE, glm::value_ptr(Mb));
        setMaterial(uBaseColor_, uEmissive_, uKa_, uKd_, uKs_, uShine_, cat_.color, 0.f);
        drawMesh(cyl_);

        glm::mat4 Mh(1.f); Mh = glm::translate(Mh, p + glm::vec3(0.f, cat_.size.y, 0.f)); Mh = glm::scale(Mh, {0.8f,0.8f,0.8f});
        glUniformMatrix4fv(uModel_,1,GL_FALSE, glm::value_ptr(Mh));
        setMaterial(uBaseColor_, uEmissive_, uKa_, uKd_, uKs_, uShine_, cat_.color, 0.f);
        drawMesh(cone_);

        for (float s : {-0.35f, 0.35f}) {
            glm::mat4 Me(1.f); Me = glm::translate(Me, p + glm::vec3(s, cat_.size.y + 0.4f, 0.f)); Me = glm::scale(Me, {0.25f,0.25f,0.25f});
            glUniformMatrix4fv(uModel_,1,GL_FALSE, glm::value_ptr(Me));
            setMaterial(uBaseColor_, uEmissive_, uKa_, uKd_, uKs_, uShine_, cat_.color, 0.f);
            drawMesh(cone_);
        }
        glm::mat4 Mt(1.f); Mt = glm::translate(Mt, p + glm::vec3(0.f, 0.3f, 0.55f)); Mt = glm::scale(Mt, {0.12f,1.0f,0.12f});
        glUniformMatrix4fv(uModel_,1,GL_FALSE, glm::value_ptr(Mt));
        setMaterial(uBaseColor_, uEmissive_, uKa_, uKd_, uKs_, uShine_, {1.0f,0.7f,0.4f}, 0.f);
        drawMesh(cyl_);
    }
}
