#include "particle_system.hpp"
#include <stdexcept> 
#include <algorithm> 
#include <iostream>  
#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"

ParticleSystem::ParticleSystem(const std::string& texturePath, const std::string& vertShaderPath, const std::string& fragShaderPath)
    : particles(MAX_PARTICLES), randomGenerator(std::random_device{}())
{
    try {
        shader = std::make_unique<Shader>(vertShaderPath.c_str(), fragShaderPath.c_str());
        texture = std::make_unique<Texture2D>(texturePath.c_str());
    } catch (const std::runtime_error& e) {
        std::cerr << "ERROR::PARTICLESYSTEM::CONSTRUCTION_FAILED\n" << e.what() << std::endl;
        shader = nullptr;
        texture = nullptr;
        return; 
    }
    initBuffers();
}

ParticleSystem::~ParticleSystem() {
    if (vao != 0) glDeleteVertexArrays(1, &vao);
    if (vbo != 0) glDeleteBuffers(1, &vbo);
  
}

void ParticleSystem::initBuffers() {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * sizeof(Particle), particles.data(), GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, pos));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, currentColor));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, size));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, life));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void ParticleSystem::spawnInitialBurst(const glm::vec3& centerPos) {
     std::cout << "Spawning " << MAX_PARTICLES << " particles..." << std::endl;
    for (int i = 0; i < MAX_PARTICLES; ++i) {
        respawnParticle(particles[i], centerPos);
    }
    lastUsedParticle = 0;
   
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, MAX_PARTICLES * sizeof(Particle), particles.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
     std::cout << "Spawn complete." << std::endl;
}


int ParticleSystem::findUnusedParticle() {
    // from last used particle onwards
    for (int i = lastUsedParticle; i < MAX_PARTICLES; ++i) {
        if (particles[i].life <= 0.0f) {
            lastUsedParticle = i;
            return i;
        }
    }
    // else we have to search from the beginning
    for (int i = 0; i < lastUsedParticle; ++i) {
        if (particles[i].life <= 0.0f) {
            lastUsedParticle = i;
            return i;
        }
    }
    // the first particle is overwritten
    lastUsedParticle = 0;
    return 0;
}


void ParticleSystem::respawnParticle(Particle& particle, const glm::vec3& centerPos) {
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    particle.isSecondary = (dist(randomGenerator) < SECONDARY_PARTICLE_FRACTION);

    glm::vec3 dir;
    float phi = dist(randomGenerator) * 2.0f * glm::pi<float>();
    float costheta, theta, sinTheta;

    float biasRoll = dist(randomGenerator);
    if (biasRoll < DENSITY_BIAS_FACTOR) {
        costheta = (dist(randomGenerator) * 2.0f - 1.0f) * 0.5f;
    } else {
        costheta = dist(randomGenerator) * 2.0f - 1.0f;
    }
    theta = acos(costheta);
    sinTheta = sin(theta);
    dir = glm::vec3(sinTheta * cos(phi), sinTheta * sin(phi), costheta);

    float speed;
    if (particle.isSecondary) {
        speed = MIN_INITIAL_SPEED + dist(randomGenerator) * (MAX_INITIAL_SPEED * 0.4f - MIN_INITIAL_SPEED);
    } else {
        float speedFactor = 0.4f + dist(randomGenerator) * 0.6f;
        speed = MIN_INITIAL_SPEED + speedFactor * (MAX_INITIAL_SPEED - MIN_INITIAL_SPEED);
    }
    particle.velocity = dir * speed;

    particle.pos = centerPos + dir * EXPLOSION_RADIUS * (0.5f + dist(randomGenerator) * 0.5f);

    float r_base, g_base, b_base;
    if (particle.isSecondary) {
        r_base = 0.8f + dist(randomGenerator) * 0.7f; g_base = 0.2f + dist(randomGenerator) * 0.4f; b_base = 0.1f + dist(randomGenerator) * 0.5f;
        particle.initialColor = glm::vec4(r_base, g_base, b_base, 0.8f);
    } else {
        r_base = 1.5f + dist(randomGenerator) * 1.0f; g_base = 1.2f + dist(randomGenerator) * 1.3f;
        b_base = dist(randomGenerator) < 0.7f ? (0.5f + dist(randomGenerator) * 1.0f) : (1.5f + dist(randomGenerator) * 1.0f);
        particle.initialColor = glm::vec4(r_base, g_base, b_base, 1.0f);
    }
    particle.currentColor = particle.initialColor;

    particle.size = 1.0f + dist(randomGenerator) * 2.0f;
    if (particle.isSecondary) particle.size *= 1.2f;

    particle.lifetime = particle.isSecondary ? PARTICLE_LIFETIME_LONG : PARTICLE_LIFETIME_BASE;
    particle.lifetime *= (0.8f + dist(randomGenerator) * 0.4f);
    particle.life = 1.0f; //full life
}


void ParticleSystem::update(float dt, float currentTime, const glm::vec3& centerPos) {
    if (!shader || !texture) return; 

    int activeCount = 0;
    for (int i = 0; i < MAX_PARTICLES; ++i) {
        Particle& p = particles[i];

        if (p.life > 0.0f) {
            activeCount++;
            p.life -= dt / p.lifetime;

            if (p.life > 0.0f) {
                // turbulence 
                glm::vec3 noisePos = p.pos * TURBULENCE_FREQUENCY + currentTime * TURBULENCE_TIME_FACTOR;
                // using perlin noise (noise output would be in the range [-1, 1] we can scale that)
                float nX = stb_perlin_noise3(noisePos.x, noisePos.y, noisePos.z, 0, 0, 0);
                float nY = stb_perlin_noise3(noisePos.y, noisePos.z, noisePos.x, 0, 0, 0);
                float nZ = stb_perlin_noise3(noisePos.z, noisePos.x, noisePos.y, 0, 0, 0);
                glm::vec3 turbulence = glm::vec3(nX, nY, nZ) * TURBULENCE_STRENGTH * (1.0f - p.life); // Weaken over time

              
                glm::vec3 acceleration = turbulence;
                float currentDrag = DRAG_FACTOR * (p.isSecondary ? 1.5f : 1.0f);
                acceleration -= p.velocity * currentDrag; // opposing the velocity
                p.velocity += acceleration * dt;
                p.pos += p.velocity * dt;

               
                float lifeFactor = p.life;
                glm::vec4 targetColor = p.isSecondary ? glm::vec4(0.5f, 0.1f, 0.1f, 0.1f) : glm::vec4(0.7f, 0.2f, 0.4f, 0.2f);
                p.currentColor = glm::mix(targetColor, p.initialColor, lifeFactor);
                p.currentColor.a = glm::mix(targetColor.a, p.initialColor.a, pow(lifeFactor, 0.5f));

            } else { // dead particles
                p.life = 0.0f; 
                p.currentColor.a = 0.0f;
            }
        }
    }

    // Update GPU buffer
    if (activeCount > 0 || true) { 
                                  
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, MAX_PARTICLES * sizeof(Particle), particles.data());
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void ParticleSystem::render(const glm::mat4& view, const glm::mat4& projection) {
    if (!shader || !texture || vao == 0) return; 

    shader->use();
    shader->setMat4("projection", projection);
    shader->setMat4("view", view);
    shader->setFloat("uFadeExponent", FADE_EXPONENT);
    shader->setFloat("uSizeFadeExponent", SIZE_FADE_EXPONENT);
    shader->setFloat("uBasePointSize", PARTICLE_BASE_POINT_SIZE);
    shader->setInt("particleTexture", 0);

    texture->bind(GL_TEXTURE0);

    glBindVertexArray(vao);
    glDrawArrays(GL_POINTS, 0, MAX_PARTICLES); 
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);
}

bool ParticleSystem::isActive() const {
    for(const auto& p : particles) {
        if (p.life > 0.0f) {
            return true;
        }
    }
    return false;
}