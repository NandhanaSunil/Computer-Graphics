#ifndef _GL_FRAMEWORK_HPP_
#define _GL_FRAMEWORK_HPP_

#include "common.hpp"
namespace glFramework
{
    void initGL(void);
    void error_callback(int error, const char* description);
    void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    // void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
    void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    void processInput(GLFWwindow* window);
}
#endif