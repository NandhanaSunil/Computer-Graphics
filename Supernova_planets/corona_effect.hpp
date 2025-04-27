#ifndef CORONA_EFFECT_H
#define CORONA_EFFECT_H

#include <memory>
#include <string>
#include <ctime>

#define GLEW_STATIC
#include <GL/glew.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/compatibility.hpp>

#include "config.hpp"
#include "shader.hpp"
#include "texture.hpp"

class CoronaEffect {
public:
    CoronaEffect(const std::string& texturePath, const std::string& vertShaderPath, const std::string& fragShaderPath);
    ~CoronaEffect();

    // void update(float dt, float currentTime, SimulationState state, float explosionStartTime);
    void update(float dt, float currentTime, SimulationState state, float triggerTime);
    void render(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& centerPos);
    bool isVisible() const;

private:
    GLuint vao = 0, vbo = 0;
    std::unique_ptr<Shader> shader;
    std::unique_ptr<Texture2D> texture;

    float currentBaseSize = CORONA_BASE_SIZE;
    float currentBrightness = CORONA_INITIAL_BRIGHTNESS;

    void initBuffers();
};

#endif // CORONA_EFFECT_H