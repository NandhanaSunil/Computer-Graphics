#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include <vector>
#include <random>
#include <memory> 

#define GLEW_STATIC
#include <GL/glew.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/compatibility.hpp> 

#include "config.hpp"
#include "shader.hpp"
#include "texture.hpp"

struct Particle {
    glm::vec3 pos;
    glm::vec3 velocity;
    glm::vec4 initialColor;
    glm::vec4 currentColor;
    float size;
    float life;         
    float lifetime;     
    bool isSecondary;  

    Particle() : pos(0.0f), velocity(0.0f), initialColor(1.0f), currentColor(1.0f),
                 size(1.0f), life(0.0f), lifetime(PARTICLE_LIFETIME_BASE), isSecondary(false) {}
};


class ParticleSystem {
public:
    ParticleSystem(const std::string& texturePath, const std::string& vertShaderPath, const std::string& fragShaderPath);
    ~ParticleSystem();

    void update(float dt, float currentTime, const glm::vec3& centerPos);
    void render(const glm::mat4& view, const glm::mat4& projection);
    void spawnInitialBurst(const glm::vec3& centerPos);
    bool isActive() const; // to check the liveness of particles 

private:
    std::vector<Particle> particles;
    GLuint vao = 0, vbo = 0;
    int lastUsedParticle = 0;
    std::unique_ptr<Shader> shader;
    std::unique_ptr<Texture2D> texture;
    std::mt19937 randomGenerator; // mersenne Twister random generation (better randomness)

    void initBuffers();
    int findUnusedParticle();
    void respawnParticle(Particle& particle, const glm::vec3& centerPos);
};

#endif // PARTICLE_SYSTEM_H