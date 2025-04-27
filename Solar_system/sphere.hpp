#ifndef SPHERE_HPP
#define SPHERE_HPP

#include <vector>
#include "common.hpp"

#include <GL/glew.h>
#include <string>

class Sphere {
public:
    GLuint vao, vbo, ebo, texVBO;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    std::vector<float> texCoords;
    
    Sphere(float radius, int sectorCount, int stackCount);
    void draw(GLint modelLoc, glm::mat4 model, GLuint texture);
    GLuint loadTexture(const std::string& filePath) ;
    ~Sphere();
    
private:
    void generateSphere(float radius, int sectorCount, int stackCount);
    
};

void loadTexture();
#endif // SPHERE_H
