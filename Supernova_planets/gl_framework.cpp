#include "gl_framework.hpp"

namespace glFramework
{
  //! Initialize GL State
  void initGL(void)
  {
    //Set framebuffer clear color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    //Set depth buffer furthest depth
    glClearDepth(1.0);
    //Set depth test to less-than
    glDepthFunc(GL_LESS);
    //Enable depth testing
    glEnable(GL_DEPTH_TEST); 
    //Enable Gourard shading
    glShadeModel(GL_SMOOTH);
  }
  //!GLFW Error Callback
  void error_callback(int error, const char* description)
  {
    std::cerr<<description<<std::endl;
  }
  //!GLFW framebuffer resize callback
  void framebuffer_size_callback(GLFWwindow* window, int width, int height)
  {
    //!Resize the viewport to fit the window size - draw to entire window
    glViewport(0, 0, width, height);
  }
  //!GLFW keyboard callback
    // void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
    // {
    //     //!Close the window if the ESC key was pressed
    //     if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    //     glfwSetWindowShouldClose(window, GL_TRUE);
    // }
    //!GLFW mouse button callback
    void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        {
            // Handle left mouse button press to move camera to left
            
        }
        else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
        {
            // Handle right mouse button press
        }
    }
    //!GLFW cursor position callback
    void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
    {
        // Handle mouse movement
    }
    //!GLFW scroll callback
    void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
    {
        // Handle mouse scroll
    }
    //!GLFW process input
    void processInput(GLFWwindow* window)
    {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            // Handle W key press
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            // Handle S key press
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            // Handle A key press
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            // Handle D key press
        }
    }
}
