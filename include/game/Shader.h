#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader {
public:
    GLuint ID;

    Shader() : ID(0) {}

    ~Shader() {
        if (ID) glDeleteProgram(ID);
    }

    // Load shaders from file paths
    bool LoadFromFiles(const std::string& vertexPath, const std::string& fragmentPath) {
        std::string vertexCode = ReadFile(vertexPath);
        std::string fragmentCode = ReadFile(fragmentPath);

        if (vertexCode.empty() || fragmentCode.empty()) {
            return false;
        }

        return CompileAndLink(vertexCode.c_str(), fragmentCode.c_str());
    }

    // Load shaders from string sources
    bool LoadFromStrings(const char* vertexSource, const char* fragmentSource) {
        return CompileAndLink(vertexSource, fragmentSource);
    }

    void Use() const {
        glUseProgram(ID);
    }

    // Utility uniform functions
    void SetBool(const std::string& name, bool value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }

    void SetInt(const std::string& name, int value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }

    void SetFloat(const std::string& name, float value) const {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }

    void SetVec3(const std::string& name, const glm::vec3& value) const {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
    }

    void SetVec4(const std::string& name, const glm::vec4& value) const {
        glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
    }

    void SetMat4(const std::string& name, const glm::mat4& mat) const {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
    }

private:
    std::string ReadFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "ERROR: Could not open shader file: " << path << std::endl;
            return "";
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    bool CompileAndLink(const char* vertexSource, const char* fragmentSource) {
        GLuint vertex = CompileShader(GL_VERTEX_SHADER, vertexSource);
        if (vertex == 0) return false;

        GLuint fragment = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);
        if (fragment == 0) {
            glDeleteShader(vertex);
            return false;
        }

        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);

        GLint success;
        glGetProgramiv(ID, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(ID, 512, NULL, infoLog);
            std::cerr << "ERROR: Shader program linking failed\n" << infoLog << std::endl;
            glDeleteShader(vertex);
            glDeleteShader(fragment);
            return false;
        }

        glDeleteShader(vertex);
        glDeleteShader(fragment);
        return true;
    }

    GLuint CompileShader(GLenum type, const char* source) {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, NULL);
        glCompileShader(shader);

        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            std::cerr << "ERROR: Shader compilation failed ("
                << (type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT")
                << ")\n" << infoLog << std::endl;
            glDeleteShader(shader);
            return 0;
        }

        return shader;
    }
};

#endif // SHADER_H