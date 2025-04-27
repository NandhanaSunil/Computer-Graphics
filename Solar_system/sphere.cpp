
#include "sphere.hpp"
#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "stb_image.h"
#include <iostream>

Sphere::Sphere(float radius, int sectorCount, int stackCount) {
  generateSphere(radius, sectorCount, stackCount);

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);
  glGenBuffers(1, &texVBO);

  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0); 
  glEnableVertexAttribArray(0);
  
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float))); 
  glEnableVertexAttribArray(1);

  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float))); 
  glEnableVertexAttribArray(2);

  glBindBuffer(GL_ARRAY_BUFFER, texVBO);
  glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(float), &texCoords[0], GL_STATIC_DRAW);
  glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0); 
  glEnableVertexAttribArray(3);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

  glBindVertexArray(0);
}

void Sphere::draw(GLint modelLoc, glm::mat4 model, GLuint texture) {
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);
  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
  glBindVertexArray(vao);
  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}

GLuint Sphere::loadTexture(const std::string& filePath) {
  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  int width, height, nrChannels;
  unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);
  if (data) {
      GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
      glGenerateMipmap(GL_TEXTURE_2D);
  } else {
      std::cout << "Failed to load texture: " << filePath << std::endl;
  }
  stbi_image_free(data);
  
  return texture;
}

void Sphere::generateSphere(float radius, int sectorCount, int stackCount) {
  vertices.clear();
  indices.clear();

  for (int i = 0; i <= stackCount; ++i) {
      float stackAngle = glm::pi<float>() * float(i) / float(stackCount);
      float xy = radius * sin(stackAngle);
      float z = radius * cos(stackAngle);

      for (int j = 0; j <= sectorCount; ++j) {
          float sectorAngle = 2 * glm::pi<float>() * float(j) / float(sectorCount);
          float x = xy * cos(sectorAngle);
          float y = xy * sin(sectorAngle);

          vertices.push_back(x);
          vertices.push_back(y);
          vertices.push_back(z);

          glm::vec3 normal = glm::normalize(glm::vec3(x, y, z));
          vertices.push_back(normal.x);
          vertices.push_back(normal.y);
          vertices.push_back(normal.z);

          float red = fabs(sin(sectorAngle));
          float green = fabs(cos(stackAngle));
          float blue = fabs(sin(stackAngle) * cos(sectorAngle));
          vertices.push_back(red);
          vertices.push_back(green);
          vertices.push_back(blue);

          float u = (((float)j / sectorCount)); 
          float v = ((float)i / stackCount); 

          texCoords.push_back(u);
          texCoords.push_back(v);
      }
  }

  for (int i = 0; i < stackCount; ++i) {
      int k1 = i * (sectorCount + 1);
      int k2 = k1 + sectorCount + 1;
      
      for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
          indices.push_back(k1);
          indices.push_back(k2);
          indices.push_back(k1 + 1);

          indices.push_back(k1 + 1);
          indices.push_back(k2);
          indices.push_back(k2 + 1);
      }
  }
}







