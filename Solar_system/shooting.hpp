#ifndef SHOOTING_HPP
#define SHOOTING_HPP
#include "common.hpp"

// structure for a shooting star
struct Particle {
    glm::vec3 position;
    float life;
    float maxLife;
    float size;
    glm::vec3 velocity; // velocity of the particle
};

void initParticleSystem();
void spawnShootingStar();
void updateParticles(float deltaTime);
void renderParticles(const glm::mat4& view, const glm::mat4& projection);
GLuint compileShader(GLenum type, const std::string& filename);
void initShaders(const std::string& vertexShaderSource, const std::string& fragmentShaderSource);

#endif // SHOOTING_HPP