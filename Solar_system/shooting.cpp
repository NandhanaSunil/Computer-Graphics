#include "shooting.hpp"

std::vector<Particle> particles;
GLuint particleVAO, particleVBO;
GLuint particleShaderProgram;

const size_t MAX_PARTICLES = 500;
const int TRAIL_LENGTH = 200;
const float TRAIL_SPACING = 0.02f; // distance between trail particles

void initParticleSystem() {
    glGenVertexArrays(1, &particleVAO);
    glGenBuffers(1, &particleVBO);
    
    glBindVertexArray(particleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
    glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * 5 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(4 * sizeof(float)));
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void spawnShootingStar() {
    glm::vec3 startPos(
        (rand() % 20 - 10) * 5.5f, // (-5 to 5)
        5.0f,                      
        (rand() % 10 - 5) * 5.5f   
    );
    
    // generate random directions for velocity
    float angle = (rand() % 360) * (M_PI / 180.0f);
    float zvelocity = rand() % 2 > 0 ? 1.0f : -1.0f;
    float xvelocity = rand() % 2 > 0 ? 1.0f : -1.0f;
    float yvelocity = rand() % 2 > 0 ? 1.0f : -1.0f;
    glm::vec3 velocity(-5.20f*xvelocity, -0.5f*sin(angle)*yvelocity, 0.5f* zvelocity);
    float lifetime = 3.5f + (rand() % 100) * 0.01f;
    
    // trail particles
    for (int i = 0; i < TRAIL_LENGTH; i++) {
        Particle p;
        p.position = startPos - velocity * (i * TRAIL_SPACING);
        p.maxLife = lifetime * 0.8f * (1.0f - (float)i/TRAIL_LENGTH);
        p.life = p.maxLife;
        p.size = 8.0f * (1.0f - (float)i/(TRAIL_LENGTH*2));
        p.velocity = velocity;
        particles.push_back(p);
    }
    
    // main star 
    Particle mainStar;
    mainStar.position = startPos;
    mainStar.maxLife = lifetime;
    mainStar.life = lifetime;
    mainStar.size = 12.0f;
    particles.push_back(mainStar);
}

void updateParticles(float deltaTime) {
    // glm::vec3 velocity(-50.20f, -0.5f, 0.0f);
    // glm::vec3 velocity;
    
    for (auto it = particles.begin(); it != particles.end(); ) {
        // get unit velocity in the direction of particles
        // glm::vec3 unitVelocity = glm::normalize(it->position - velocity);
        glm::vec3 uvelocity = 5.0f * glm::normalize(it->velocity);
        it->position += uvelocity * deltaTime;
        it->life -= deltaTime;
        
        if (it->life <= 0.0f) {
            it = particles.erase(it);
        } else {
            ++it;
        }
    }
    
    // spawn new stars randomly
    if (abs(rand()) % 700 < 2 && particles.size() < MAX_PARTICLES - TRAIL_LENGTH - 1) {
        spawnShootingStar();
    }
}

void renderParticles(const glm::mat4& view, const glm::mat4& projection) {
    glUseProgram(particleShaderProgram);
    // glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glUniformMatrix4fv(glGetUniformLocation(particleShaderProgram, "view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(particleShaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
    
    std::vector<float> particleData;
    for (const auto& p : particles) {
        particleData.push_back(p.position.x);
        particleData.push_back(p.position.y);
        particleData.push_back(p.position.z);
        particleData.push_back(p.life / p.maxLife); 
        particleData.push_back(p.size);            
    }
    
    glBindVertexArray(particleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, particleData.size() * sizeof(float), particleData.data());
    
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glDrawArrays(GL_POINTS, 0, particles.size());
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
    // glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
}

#include <fstream>
#include <sstream>

GLuint compileShader(GLenum type, const std::string& filename) {
    std::ifstream shaderFile(filename);
    if (!shaderFile.is_open()) {
        std::cerr << "Failed to open shader file: " << filename << std::endl;
        return 0;
    }

    std::stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    std::string shaderCode = shaderStream.str();
    const char* shaderSource = shaderCode.c_str();

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &shaderSource, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed (" << filename << "):\n" << infoLog << std::endl;
    }

    return shader;
}


void initShaders(const std::string& vertexShaderSource, const std::string& fragmentShaderSource) {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    
    particleShaderProgram = glCreateProgram();
    glAttachShader(particleShaderProgram, vertexShader);
    glAttachShader(particleShaderProgram, fragmentShader);
    glLinkProgram(particleShaderProgram);
    
    GLint success;
    glGetProgramiv(particleShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(particleShaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed:\n" << infoLog << std::endl;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}
