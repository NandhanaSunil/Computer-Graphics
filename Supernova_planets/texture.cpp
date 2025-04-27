#include "texture.hpp"
#include "stb_image.h"

Texture2D::Texture2D(const char* path, bool flip) : ID(0), Width(0), Height(0), NrChannels(0) {
    glGenTextures(1, &ID);
    stbi_set_flip_vertically_on_load(flip);
    unsigned char *data = stbi_load(path, &Width, &Height, &NrChannels, 0);
    if (data) {
        GLenum internalFormat = GL_RGBA;
        GLenum dataFormat = GL_RGBA;
        if (NrChannels == 1) { internalFormat = GL_RED; dataFormat = GL_RED; }
        else if (NrChannels == 3) { internalFormat = GL_RGB; dataFormat = GL_RGB; }
        else if (NrChannels == 4) { internalFormat = GL_RGBA; dataFormat = GL_RGBA; }
        else {
            std::cerr << "Unsupported channel count: " << NrChannels << " for texture " << path << std::endl;
            stbi_image_free(data);
            glDeleteTextures(1, &ID); 
            ID = 0; 
            throw std::runtime_error("Unsupported texture format");
        }

        glBindTexture(GL_TEXTURE_2D, ID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, Width, Height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        std::cout << "Texture loaded: " << path << " (" << Width << "x" << Height << ", " << NrChannels << " channels)" << std::endl;
        stbi_image_free(data);
    } else {
        std::cerr << "Texture failed to load at path: " << path << std::endl;
        glDeleteTextures(1, &ID); 
        ID = 0; // invalid
        throw std::runtime_error("Failed to load texture");
    }
    glBindTexture(GL_TEXTURE_2D, 0); // unbind
}

Texture2D::~Texture2D() {
    if (ID != 0) {
        glDeleteTextures(1, &ID);
    }
}

void Texture2D::bind(GLenum textureUnit) const {
    if (ID != 0) {
        glActiveTexture(textureUnit);
        glBindTexture(GL_TEXTURE_2D, ID);
    } else {

         glActiveTexture(textureUnit);
         glBindTexture(GL_TEXTURE_2D, 0); // nothing to bind
    }
}