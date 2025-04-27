#include "corona_effect.hpp"


#include <stdexcept>
#include <iostream>
#include <algorithm> 
#include <GLFW/glfw3.h>

CoronaEffect::CoronaEffect(const std::string& texturePath, const std::string& vertShaderPath, const std::string& fragShaderPath) {
     try {
        shader = std::make_unique<Shader>(vertShaderPath.c_str(), fragShaderPath.c_str());
        texture = std::make_unique<Texture2D>(texturePath.c_str());
    } catch (const std::runtime_error& e) {
        std::cerr << "ERROR::CORONAEFFECT::CONSTRUCTION_FAILED\n" << e.what() << std::endl;
        shader = nullptr;
        texture = nullptr;
        return;
    }
    initBuffers();
}

CoronaEffect::~CoronaEffect() {
    if (vao != 0) glDeleteVertexArrays(1, &vao);
    if (vbo != 0) glDeleteBuffers(1, &vbo);
}

void CoronaEffect::initBuffers() {
    float vertices[] = { // rendering it as a quad so that camera movement does not affect it much
        -0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
void CoronaEffect::update(float dt, float currentTime, SimulationState state, float triggerTime) {

    if (!shader || !texture) return;

    float baseSize = CORONA_BASE_SIZE; // start size for expansion
    float maxSize = baseSize * CORONA_MAX_EXPANSION_FACTOR; // size after expansion
    float baseBrightness = CORONA_INITIAL_BRIGHTNESS; // brightness level

    switch (state) {
        case SimulationState::STATE_PRE_EXPLOSION:
            currentBaseSize = baseSize;
            currentBrightness = baseBrightness;
            break;

        case SimulationState::STATE_CORONA_EXPANDING:
            if (triggerTime > 0.0f) {
                float timeSinceTrigger = currentTime - triggerTime;
                float expansionProgress = glm::clamp(timeSinceTrigger / CORONA_TO_PARTICLE_DELAY, 0.0f, 1.0f);
                float expansionCurve = 1.0f - pow(1.0f - expansionProgress, 3.0f); // Ease-out

                currentBaseSize = glm::mix(baseSize, maxSize, expansionCurve);
                // brightness stays constant during expansion
                currentBrightness = baseBrightness;
            } else {
                // go back to base state display
                currentBaseSize = baseSize;
                currentBrightness = baseBrightness;
            }
            break;

        case SimulationState::STATE_EXPLODING:
            // corona at its max expanded size and brightness
             if (triggerTime > 0.0f) { 
                currentBaseSize = maxSize; // hold max size
                currentBrightness = baseBrightness;
             } else {
                 
                 currentBaseSize = baseSize;
                 currentBrightness = baseBrightness;
             }
            break;

        case SimulationState::STATE_FADING:
            
             if (triggerTime > 0.0f) {
                float particleBurstTime = triggerTime + CORONA_TO_PARTICLE_DELAY;
                float fadingStartTime = particleBurstTime + EXPLOSION_TRANSITION_DURATION;

                if (currentTime >= fadingStartTime) {
                    float timeSinceFadeStart = currentTime - fadingStartTime;
                    float fadeProgress = glm::clamp(timeSinceFadeStart / CORONA_ACTUAL_FADE_DURATION, 0.0f, 1.0f);
                    float fadeCurve = pow(fadeProgress, CORONA_FADE_SPEED);

                    currentBaseSize = glm::mix(maxSize, 0.0f, fadeCurve);
                    currentBrightness = glm::mix(baseBrightness, 0.0f, fadeCurve);
                } else {
                    currentBaseSize = maxSize;
                    currentBrightness = baseBrightness;
                }
            } else {
                currentBaseSize = 0.0f;
                currentBrightness = 0.0f;
            }
            break;

        default:
            currentBaseSize = 0.0f;
            currentBrightness = 0.0f;
            break;
    }
}

bool CoronaEffect::isVisible() const {
    return currentBrightness > 0.005f; 
}

// In corona_effect.cpp -> CoronaEffect::render(...)

void CoronaEffect::render(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& centerPos) {
    if (!shader || !texture || vao == 0 || !isVisible()) return;

    shader->use();
    shader->setMat4("projection", projection);
    shader->setMat4("view", view);
    shader->setFloat("brightness", currentBrightness / (float)NUM_CORONA_LAYERS); 
    shader->setInt("coronaTexture", 0); 

    // REMOVE THESE - Likely not used by a billboard shader
    // shader->setFloat("u_time", (float)glfwGetTime());
    // shader->setVec2("u_resolution", glm::vec2(1920, 1080));

    texture->bind(GL_TEXTURE0);
    glBindVertexArray(vao);

    //  multiple layers 
    for (int i = 0; i < NUM_CORONA_LAYERS; ++i) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, centerPos); // Position

        // billboarding 
         glm::mat4 viewRotationOnly = glm::mat4(glm::mat3(view)); 
         glm::mat4 billboardMatrix = glm::inverse(viewRotationOnly); 
         // apply billboard rotation (ensure translation is done first)
         model = model * billboardMatrix;

        float angle = ((float)i / NUM_CORONA_LAYERS) * glm::pi<float>() * 2.0f;
        float time = static_cast<float>(glfwGetTime()); 
        angle += time * CORONA_LAYER_ROTATION_SPEED * (i % 2 == 0 ? 1.0f : -1.0f); 
        model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));

        // scale 
        float layerScale = currentBaseSize * (1.0f + (float)i * 0.05f); 
        model = glm::scale(model, glm::vec3(layerScale));

        shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}