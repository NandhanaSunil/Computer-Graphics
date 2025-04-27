#ifndef SPHERE_HPP
#define SPHERE_HPP

#include "common.hpp"

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

#endif // SPHERE_H
