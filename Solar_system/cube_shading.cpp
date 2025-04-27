#include "gl_framework.hpp"
#include "shader_util.hpp"
#include "sphere.hpp"
#include "stb_image.h"
#include <iostream>
#include "shooting.hpp"

// variables for rotation of view
float rot_angle_x = 0.0f;
float rot_angle_y = 0.0f;
float rot_angle_z = 0.0f;

float angle = 0.0f; // Rotation angle of earth
float revolutionAngle = 0.0f;


float semiMajorAxis = 20.0f; // a for earth
float semiMinorAxis = 18.0f; // b for earth

float moon_orbit_radius = 5.0f;

bool isPaused = false;

bool introAnimationPlaying = true;
float introStartTime = 0.0f;
const float introDuration = 15.0f;

std::vector<glm::vec3> cameraPosKeyframes = {
    glm::vec3(0.0f, 60.0f, 150.0f),   
    glm::vec3(0.0f, 8.0f, 10.0f),    // adjust this for the movement of camera in the beginning
    glm::vec3(40.0f, 40.0f, 0.0f)   
};

std::vector<glm::vec3> cameraTargetKeyframes = {
    glm::vec3(0.0f, 0.0f, 0.0f),   
    glm::vec3(0.0f, 0.0f, 0.0f),       
    glm::vec3(0.0f, 0.0f, 0.0f)        
};

float quadVertices[] = {
    // Positions   and texCoords for the background
    -1.0f,  1.0f,   0.0f, 1.0f,  
    -1.0f, -1.0f,   0.0f, 0.0f,  
     1.0f, -1.0f,   1.0f, 0.0f,  

    -1.0f,  1.0f,   0.0f, 1.0f,  
     1.0f, -1.0f,   1.0f, 0.0f,  
     1.0f,  1.0f,   1.0f, 1.0f   
};

glm::vec3 interactiveCameraPos;
glm::vec3 interactiveCameraTarget;

float cameraDistance = 70.7f;
const float minZoomDistance = 8.0f;  
const float maxZoomDistance = 200.0f; 
const float zoomSensitivity = 3.0f; 
const float panSensitivity = 2.0f; 

glm::vec3 finalViewDirection;
glm::vec3 finalCameraTarget;

//ellipse is defined by x = acos(theta) and y = bsin(theta)

float radius = 3; // earth radius
int sectorCount = 100;
int stackCount = 100;

Sphere* earth;
Sphere* moon;
Sphere* Sun;
GLuint texture;
GLuint moontexture;
GLuint suntexture;

float sun_radius = 5;
float sunRotationAngle = 0.0f;

float moon_radius = 1.0f;
float moonRotationAngle = 0.0f; // Rotation angle
float moonRevolutionAngle = 0.0f;

GLuint orbitVAO, orbitVBO;
const int ORBIT_SEGMENTS = 360; 

GLuint shaderProgram, bgShaderProgram;
GLuint vbo, vao, ebo, vbo_moon, vao_moon, ebo_moon;
GLuint orbitShaderProgram;


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (introAnimationPlaying) {
        return;
    }

    if (abs(yoffset) > 0.01) 
    {
        glm::vec3 viewDirection = interactiveCameraPos - interactiveCameraTarget;
        float currentDistance = glm::length(viewDirection);
        viewDirection = glm::normalize(viewDirection);

        float newDistance = currentDistance + (float)yoffset * zoomSensitivity;

        newDistance = glm::clamp(newDistance, minZoomDistance, maxZoomDistance);
        interactiveCameraPos = interactiveCameraTarget + viewDirection * newDistance;
        cameraDistance = newDistance;
    }

    if (abs(xoffset) > 0.01) 
    {
        glm::vec3 forward = glm::normalize(interactiveCameraTarget - interactiveCameraPos);
        glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 right = glm::normalize(glm::cross(forward, worldUp));
        glm::vec3 translation = right * (-1 *(float)xoffset) * panSensitivity;

        interactiveCameraPos += translation;
        interactiveCameraTarget += translation;
    }

    // std::cout << "Pos: " << interactiveCameraPos.x << "," << interactiveCameraPos.y << "," << interactiveCameraPos.z
    //           << " Target: " << interactiveCameraTarget.x << "," << interactiveCameraTarget.y << "," << interactiveCameraTarget.z
    //           << " Dist: " << cameraDistance << std::endl;
}


void initOrbitShaderGL(void) {
    std::string vertex_shader_file("orbit_vs.glsl");
    std::string fragment_shader_file("orbit_fs.glsl");

    std::vector<GLuint> shaderList;
    shaderList.push_back(glFramework::LoadShaderGL(GL_VERTEX_SHADER, vertex_shader_file));
    shaderList.push_back(glFramework::LoadShaderGL(GL_FRAGMENT_SHADER, fragment_shader_file));

    orbitShaderProgram = glFramework::CreateProgramGL(shaderList);
}

// initialize shaders
void initShadersGL(void) {
    std::string vertex_shader_file("ppixel_vs.glsl");
    std::string fragment_shader_file("ppixel_fs.glsl");

    std::vector<GLuint> shaderList;
    shaderList.push_back(glFramework::LoadShaderGL(GL_VERTEX_SHADER, vertex_shader_file));
    shaderList.push_back(glFramework::LoadShaderGL(GL_FRAGMENT_SHADER, fragment_shader_file));

    shaderProgram = glFramework::CreateProgramGL(shaderList);
}

void initbgShadersGL(void) {
    std::string vertex_shader_file("skybox_vs.glsl");
    std::string fragment_shader_file("skybox_fs.glsl");

    std::vector<GLuint> bgshaderList;
    bgshaderList.push_back(glFramework::LoadShaderGL(GL_VERTEX_SHADER, vertex_shader_file));
    bgshaderList.push_back(glFramework::LoadShaderGL(GL_FRAGMENT_SHADER, fragment_shader_file));

    bgShaderProgram = glFramework::CreateProgramGL(bgshaderList);
}

GLuint quadVAO, quadVBO;
void init_background() {
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
int width, height, nrChannels;
GLuint backgroundTexture;


void loadbgtex(){
    unsigned char *data = stbi_load("c2.png", &width, &height, &nrChannels, 0);
    if (data) {
        glGenTextures(1, &backgroundTexture);
        glBindTexture(GL_TEXTURE_2D, backgroundTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    }
    else {
        std::cout << "Failed to load background texture" << std::endl;
    }
}


void render_background() {
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(bgShaderProgram);
    glUniform1i(glGetUniformLocation(bgShaderProgram, "backgroundTexture"), 0);
    glBindVertexArray(quadVAO);
    glBindTexture(GL_TEXTURE_2D, backgroundTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST); 
}

// process user input for rotation
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        rot_angle_x -= 1.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        rot_angle_x += 1.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
        rot_angle_y += 1.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        rot_angle_y -= 1.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        rot_angle_z += 1.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        rot_angle_z -= 1.0f;
    }
}


// initialize the orbit visualization
void initOrbitPath() {
    std::vector<float> orbitVertices;
    
    // points along the elliptical orbit
    for (int i = 0; i <= ORBIT_SEGMENTS; i++) {
        float angle = glm::radians(float(i) * 360.0f / ORBIT_SEGMENTS);
        float x = semiMajorAxis * cos(angle);
        float z = semiMinorAxis * sin(angle);
        
        orbitVertices.push_back(x);
        orbitVertices.push_back(0.0f); 
        orbitVertices.push_back(z);
        
        // Color 
        orbitVertices.push_back(1.0f); // r
        orbitVertices.push_back(1.0f); // g
        orbitVertices.push_back(1.0f); // b
        orbitVertices.push_back(0.2f); // a 
    }
    
    glGenVertexArrays(1, &orbitVAO);
    glGenBuffers(1, &orbitVBO);
    
    glBindVertexArray(orbitVAO);
    glBindBuffer(GL_ARRAY_BUFFER, orbitVBO);
    glBufferData(GL_ARRAY_BUFFER, orbitVertices.size() * sizeof(float), orbitVertices.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void renderOrbitPath(const glm::mat4& view, const glm::mat4& projection) {
    glUseProgram(orbitShaderProgram);
    glm::mat4 orbitModel = glm::rotate(glm::mat4(1.0f), glm::radians(10.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    
    // uniforms
    GLint modelLoc = glGetUniformLocation(orbitShaderProgram, "model");
    GLint viewLoc = glGetUniformLocation(orbitShaderProgram, "view");
    GLint projectionLoc = glGetUniformLocation(orbitShaderProgram, "projection");
    
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(2.0f);
    
    // multiple offset lines for thickness
    for (float offset = -0.08f; offset <= 0.04f; offset += 0.04f) {
        glm::mat4 offsetModel = glm::translate(orbitModel, glm::vec3(0.0f, offset, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(offsetModel));
        
        glBindVertexArray(orbitVAO);
        glDrawArrays(GL_LINE_LOOP, 0, ORBIT_SEGMENTS + 1);
    }
    
    glBindVertexArray(0);
    glDisable(GL_BLEND);
}

// initialize vertex buffer
void initVertexBufferGL(void) {
    // generateSphere(radius, sectorCount, stackCount);
    // generateMoon(moon_radius, sectorCount, stackCount);
    earth = new Sphere(radius, sectorCount, stackCount);
    moon = new Sphere(moon_radius, sectorCount, stackCount);
    Sun = new Sphere(sun_radius, sectorCount, stackCount);
    texture = earth->loadTexture("earth.jpg");
    moontexture = moon->loadTexture("moon.jpg");
    suntexture = Sun->loadTexture("sun.jpg");
}

// render the scene
void renderGL(GLFWwindow* window) {
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // glEnable(GL_CULL_FACE); // Enable back-face culling
    glEnable(GL_DEPTH_TEST);
    processInput(window);

    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    GLint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
    GLint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
    GLint lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");

    GLint isMoonLoc = glGetUniformLocation(shaderProgram, "isMoon");
    GLint isSunLoc = glGetUniformLocation(shaderProgram, "isSun");

    glm::mat4 view = glm::lookAt(glm::vec3(50.0f, 50.0f, 0.0f), 
                             glm::vec3(0.0f, 0.0f, 0.0f),  
                             glm::vec3(0.0f, 1.0f, 0.0f));  

    // glm::vec3 currentCamPos; 
    // glm::vec3 currentCamTarget; 

    glm::vec3 currentCamPos;
    glm::vec3 currentCamTarget;

    if (introAnimationPlaying) {
        float currentTime = glfwGetTime();
        float elapsedTime = currentTime - introStartTime;
        float t = glm::clamp(elapsedTime / introDuration, 0.0f, 1.0f);
        t = glm::smoothstep(0.0f, 1.0f, t); // smoothen movt

        size_t numSegments = cameraPosKeyframes.size() >= 2 ? cameraPosKeyframes.size() - 1 : 0;
        if (numSegments > 0) {
            float segmentLength = 1.0f / numSegments;
            int currentSegment = (t >= 1.0f) ? (numSegments - 1) : static_cast<int>(t / segmentLength);
            currentSegment = glm::min(currentSegment, (int)numSegments - 1);
            float segmentT = (segmentLength > 0.0f) ? (t - currentSegment * segmentLength) / segmentLength : 0.0f;
            segmentT = glm::clamp(segmentT, 0.0f, 1.0f);
            currentCamPos = glm::mix(cameraPosKeyframes[currentSegment], cameraPosKeyframes[currentSegment + 1], segmentT);
            currentCamTarget = glm::mix(cameraTargetKeyframes[currentSegment], cameraTargetKeyframes[currentSegment + 1], segmentT);
        } else if (!cameraPosKeyframes.empty()) {
             currentCamPos = cameraPosKeyframes[0];
             currentCamTarget = cameraTargetKeyframes[0];
        } else {
            currentCamPos = glm::vec3(50.0f, 50.0f, 0.0f); 
            currentCamTarget = glm::vec3(0.0f, 0.0f, 0.0f);
        }

        view = glm::lookAt(currentCamPos, currentCamTarget, glm::vec3(0.0f, 1.0f, 0.0f));

        if (elapsedTime >= introDuration) {
            introAnimationPlaying = false;
            std::cout << "Intro animation finished. Mouse scroll enabled for zoom/pan." << std::endl;

            interactiveCameraPos = currentCamPos;
            interactiveCameraTarget = currentCamTarget;
            
            cameraDistance = glm::length(interactiveCameraPos - interactiveCameraTarget);
            cameraDistance = glm::clamp(cameraDistance, minZoomDistance, maxZoomDistance);
            
            if (glm::length(interactiveCameraPos - interactiveCameraTarget) > 0.01f) {
                interactiveCameraPos = interactiveCameraTarget + glm::normalize(interactiveCameraPos - interactiveCameraTarget) * cameraDistance;
            }
            view = glm::lookAt(interactiveCameraPos, interactiveCameraTarget, glm::vec3(0.0f, 1.0f, 0.0f));
        }

    } else {
        currentCamPos = interactiveCameraPos; 
        view = glm::lookAt(interactiveCameraPos, interactiveCameraTarget, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    // glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    view = glm::rotate(view, glm::radians(rot_angle_x), glm::vec3(1.0f, 0.0f, 0.0f));
    view = glm::rotate(view, glm::radians(rot_angle_y), glm::vec3(0.0f, 1.0f, 0.0f));
    view = glm::rotate(view, glm::radians(rot_angle_z), glm::vec3(0.0f, 0.0f, 1.0f));

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1500.0f / 1020.0f, 0.1f, 250.0f);

    glm::mat4 model = glm::mat4(1.0f);
    

    float x = semiMajorAxis * cos(glm::radians(revolutionAngle));
    float z = semiMinorAxis * sin(glm::radians(revolutionAngle));
    
    
    if (!isPaused){
    updateParticles(0.016f);
    }
    renderParticles(view, projection);
    
    glm::mat4 orbitTilt = glm::rotate(glm::mat4(1.0f), glm::radians(10.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    model = orbitTilt * model;  // apply tilt first
    model = glm::translate(model, glm::vec3(x, 0.0f, z));
    
    model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));

    //moon
    glm::mat4 moon_model = model;
    float moonX = moon_orbit_radius * cos(glm::radians(moonRevolutionAngle));
    float moonZ = moon_orbit_radius * sin(glm::radians(moonRevolutionAngle));


    moon_model = glm::translate(moon_model, glm::vec3(moonX, 0.0f, moonZ)); 
    moon_model = glm::rotate(moon_model, glm::radians(moonRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));

    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    glm::mat4 sun_model = glm::mat4(1.0f);
    sun_model = glm::rotate(sun_model, glm::radians(sunRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));

    glUseProgram(shaderProgram);

        glUniform3f(lightPosLoc, 0.0f, 0.0f, 0.0f);
        glUniform3f(viewPosLoc, 15.0f, 0.0f, 0.0f);
        glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        if (isMoonLoc != -1) glUniform1i(isMoonLoc, 0);
        if (isSunLoc != -1) glUniform1i(isSunLoc, 0);
        earth->draw(modelLoc, model, texture);


        if (isMoonLoc != -1) glUniform1i(isMoonLoc, 1);
        if (isSunLoc != -1) glUniform1i(isSunLoc, 0);

        moon->draw(modelLoc, moon_model, moontexture);


        if (isMoonLoc != -1) glUniform1i(isMoonLoc, 0);
        if (isSunLoc != -1) glUniform1i(isSunLoc, 1);
        Sun->draw(modelLoc, sun_model, suntexture);

        if (!isPaused){
    // angle += 1.0f;          
    angle += 0.75f;
    // revolutionAngle += 0.5f * (1.5f - abs(sin(glm::radians(revolutionAngle)))); 
    revolutionAngle += 0.375 * (1.125f - abs(sin(glm::radians(revolutionAngle))));

    // moonRotationAngle += 2.0f;  
    moonRotationAngle += 1.5f;
    // moonRevolutionAngle += 2.0f;
    moonRevolutionAngle += 1.5f;

    // sunRotationAngle += 2.0f;
    sunRotationAngle += 1.5f;
        }
    renderOrbitPath(view, projection);

}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) { 
        if (key == GLFW_KEY_I) {
            sectorCount += 1;
            stackCount += 1;
        } else if (key == GLFW_KEY_D) {
            sectorCount -= 1;
            stackCount -= 1;
        }
        else if (key == GLFW_KEY_P) {
                isPaused = !isPaused; 
                if (isPaused) {
                    std::cout << "Simulation Paused. Press 'P' to resume." << std::endl;
                } else {
                    std::cout << "Simulation Resumed. Press 'P' to pause." << std::endl;
                
            }
        }
        std::cout << "sector: " << sectorCount << " stack: " << stackCount << std::endl;
        initVertexBufferGL();
    }
}

int main(int argc, char** argv)
{
    GLFWwindow* window;

    glfwSetErrorCallback(glFramework::error_callback);

    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 

    window = glfwCreateWindow(1500, 1020, "Solar system", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
  
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        std::cerr << "GLEW Init Failed" << std::endl;
        return -1;
    }

    glfwSetKeyCallback(window, keyCallback);
    glfwSetFramebufferSizeCallback(window, glFramework::framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback); 

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    glFramework::initGL();
    // glEnable(GL_DEPTH_TEST);
    srand(time(nullptr));
    initShadersGL();
    initbgShadersGL();
    initShaders("shooting_vs.glsl", "shooting_fs.glsl");
    initOrbitShaderGL(); 
    init_background();
    loadbgtex();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    initVertexBufferGL();
    initParticleSystem();
    initOrbitPath();    
    

    interactiveCameraPos = cameraPosKeyframes.back();
    interactiveCameraTarget = cameraTargetKeyframes.back();
    cameraDistance = glm::length(interactiveCameraPos - interactiveCameraTarget);

    cameraDistance = glm::clamp(cameraDistance, minZoomDistance, maxZoomDistance);
    if (glm::length(interactiveCameraPos - interactiveCameraTarget) > 0.01f) {
         interactiveCameraPos = interactiveCameraTarget + glm::normalize(interactiveCameraPos - interactiveCameraTarget) * cameraDistance;
    }

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_DEPTH_BUFFER_BIT);
        render_background();
        renderGL(window);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
