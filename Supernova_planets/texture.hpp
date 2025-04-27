#ifndef TEXTURE_H
#define TEXTURE_H

#define GLEW_STATIC
#include <GL/glew.h>
#include <string>
#include <iostream>


class Texture2D {
public:
    GLuint ID;
    int Width, Height, NrChannels;
    Texture2D(const char* path, bool flip = true);    
    ~Texture2D();   
    void bind(GLenum textureUnit = GL_TEXTURE0) const;
};

#endif 