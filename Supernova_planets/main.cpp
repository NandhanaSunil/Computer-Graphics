
#include "gl_framework.hpp"
#include "shader_util.hpp"
#include "sphere.hpp"

#include <string>
#include <sstream>
#include <memory>
#include <stdexcept>
#include <chrono>
#include <cmath>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define GLM_ENABLE_EXPERIMENTAL 

#define GLM_ENABLE_EXPERIMENTAL 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/compatibility.hpp>


#include "config.hpp"

#include "texture.hpp" 
#include "particle_system.hpp"
#include "corona_effect.hpp"


struct OrbitData {
    GLuint vao = 0;
    GLuint vbo = 0;
    int vertexCount = 0;
};

bool isPaused = false;
OrbitData mercuryOrbit, venusOrbit, earthOrbit, marsOrbit, jupiterOrbit, saturnOrbit, uranusOrbit, neptuneOrbit;

GLuint orbitShaderProgram;
GLint orbitMVPLoc = -1; 
GLint orbitColorLoc = -1;


float orbitAlpha = 1.0f; 
bool fadingOrbits = false;
float orbitFadeDuration = 10.0f;
float orbitFadeTimer = 0.0f;

std::vector<glm::vec3> generateEllipseVertices(float a, float b, int numSegments = 100) {
    std::vector<glm::vec3> vertices;
    vertices.reserve(numSegments);
    float deltaAngle = 2.0f * M_PI / (float)numSegments;
    for (int i = 0; i < numSegments; ++i) {
        float angle = (float)i * deltaAngle;
        vertices.emplace_back(a * cos(angle), 0.0f, b * sin(angle));
    }
    return vertices;
}

 
bool finalZoomKeyframeAdded = false; 
const float finalZoomDelay = 15.0f;
const float finalZoomDuration = 6.0f;
const glm::vec3 finalZoomTargetPos = glm::vec3(0.0f, 10.1f, 7.5f);
const glm::vec3 finalZoomLookAt = glm::vec3(0.0f);

GLuint planetShaderProgram;

// for the explosion purposes
std::unique_ptr<ParticleSystem> particleSystem = nullptr;
std::unique_ptr<CoronaEffect> coronaEffect = nullptr;

// planets
std::unique_ptr<Sphere> Sun = nullptr; GLuint suntexture;
std::unique_ptr<Sphere> mercury = nullptr; GLuint mercurytexture;
std::unique_ptr<Sphere> venus = nullptr; GLuint venustexture;
std::unique_ptr<Sphere> earth = nullptr; GLuint earthtexture;
std::unique_ptr<Sphere> mars = nullptr; GLuint marstexture;
std::unique_ptr<Sphere> jupiter = nullptr; GLuint jupitertexture;
std::unique_ptr<Sphere> saturn = nullptr; GLuint saturntexture;
std::unique_ptr<Sphere> uranus = nullptr; GLuint uranustexture;
std::unique_ptr<Sphere> neptune = nullptr; GLuint neptunetexture;
std::unique_ptr<Sphere> moon = nullptr; GLuint moontexture;

// flags for planets
bool mercuryRender = true;
bool venusRender = true;



// disappearing delays
const float MERCURY_DISAPPEAR_DELAY = 1.0f;
const float VENUS_DISAPPEAR_DELAY = 4.5f;


float sun_radius = 1.5f; float sunRotationAngle = 0.0f; float sunGrowthFactor = 1.0f;
float currentSunScaledRadius = sun_radius;

float mercury_radius = 0.5f; float mercuryRotationAngle = 0.0f; float mercuryRevolutionAngle = 0.0f; float merc_a = 5.0f; float merc_b = 4.0f; bool mercuryEscaping = false; float mercuryEscapeTime = 0.0f; glm::vec3 mercuryStartPos;
float venus_radius = 0.8f; float venusRotationAngle = 0.0f; float venusRevolutionAngle = 40.0f; float ven_a = 10.0f; float ven_b = 7.0f; bool venusEscaping = false; float venusEscapeTime = 0.0f; glm::vec3 venusStartPos; // Added escape state for Venus too
float earth_radius = 1.0f; float earthRotationAngle = 0.0f; float earthRevolutionAngle = 20.0f; float earth_a = 14.0f; float earth_b = 11.0f; bool earthEscaping = false; float earthEscapeTime = 0.0f; glm::vec3 earthStartPos;
float mars_radius = 0.9f; float marsRotationAngle = 0.0f; float marsRevolutionAngle = 100.0f; float mars_a = 18.0f; float mars_b = 15.0f; bool marsEscaping = false; float marsEscapeTime = 0.0f; glm::vec3 marsStartPos;
float jupiter_radius = 1.3f; float jupiterRotationAngle = 0.0f; float jupiterRevolutionAngle = 140.0f; float jup_a = 23.0f; float jup_b = 19.0f; bool jupiterEscaping = false; float jupiterEscapeTime = 0.0f; glm::vec3 jupiterStartPos;
float saturn_radius = 1.2f; float saturnRotationAngle = 0.0f; float saturnRevolutionAngle = 60.0f; float sat_a = 27.0f; float sat_b = 22.0f; bool saturnEscaping = false; float saturnEscapeTime = 0.0f; glm::vec3 saturnStartPos; // Increased orbit slightly
float uranus_radius = 1.1f; float uranusRotationAngle = 0.0f; float uranusRevolutionAngle = -120.0f; float uran_a = 31.0f; float uran_b = 26.0f; bool uranusEscaping = false; float uranusEscapeTime = 0.0f; glm::vec3 uranusStartPos; // Increased orbit slightly
float neptune_radius = 1.1f; float neptuneRotationAngle = 0.0f; float neptuneRevolutionAngle = 80.0f; float nept_a = 35.0f; float nept_b = 29.0f; bool neptuneEscaping = false; float neptuneEscapeTime = 0.0f; glm::vec3 neptuneStartPos; // Increased orbit slightly
float moon_radius = 0.3f;
float moon_orbit_radius = 2.5f;
float moonRotationAngle = 0.0f; float moonRevolutionAngle = 80.0f; bool moonEscaping = false; float moonEscapeTime = 0.0f; glm::vec3 moonStartPos;

// time to delay the escaping of planets (without gravitational pull)
const float PLANET_ESCAPE_DELAY = 10.0f; 
const float PLANET_ESCAPE_SPEED_MULTIPLIER = 0.05f;
bool planetsEscapeTriggered = false;


SimulationState currentState = SimulationState::STATE_PRE_EXPLOSION;
float coronaStartTime = -1.0f;
float explosionStartTime = -1.0f;
float lastFrameTime = 0.0f;


struct CameraKeyframe {
    glm::vec3 position;
    glm::vec3 lookAt;
    float durationToReach;
};

std::vector<CameraKeyframe> cameraPath;
bool isPathAnimationDone = false;
int currentPathSegment = 0;
float segmentElapsedTime = 0.0f;


void initializeCameraPath();

// this is for calculating beizer curves
glm::vec3 calculateCubicBezier(float t, const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3) {
    t = glm::clamp(t, 0.0f, 1.0f);
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    glm::vec3 p = uuu * p0;  // (1-t)^3 * P0
    p += 3.0f * uu * t * p1; // 3 * (1-t)^2 * t * P1
    p += 3.0f * u * tt * p2; // 3 * (1-t) * t^2 * P2
    p += ttt * p3;           // t^3 * P3
    return p;
}


void processInput(GLFWwindow *window, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    const glm::vec3 explosionCenter(0.0f);
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        isPaused = !isPaused;
        std::cout << "Simulation " << (isPaused ? "PAUSED" : "RESUMED") << std::endl;

        // IMPORTANT: Reset lastFrameTime when unpausing to avoid a large deltaTime jump
        if (!isPaused) {
        }
    }
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }
        else if (key == GLFW_KEY_S && currentState == SimulationState::STATE_PRE_EXPLOSION) {
            currentState = SimulationState::STATE_CORONA_EXPANDING;
            coronaStartTime = static_cast<float>(glfwGetTime());
            planetsEscapeTriggered = false;
            std::cout << "STATE: CORONA EXPANDING triggered at time " << coronaStartTime << std::endl;
            finalZoomKeyframeAdded = false;}
    }
}



void initPlanetShadersGL(void){
    std::string vertex_shader_file("main_vs.glsl");
    std::string fragment_shader_file("main_fs.glsl");

    std::vector<GLuint> shaderList;
    shaderList.push_back(glFramework::LoadShaderGL(GL_VERTEX_SHADER, vertex_shader_file));
    shaderList.push_back(glFramework::LoadShaderGL(GL_FRAGMENT_SHADER, fragment_shader_file));

    planetShaderProgram = glFramework::CreateProgramGL(shaderList);
}

void initPlanetVertexBuffers(){
    try {
        Sun = std::make_unique<Sphere>(sun_radius, 100, 100);
        suntexture = Sun->loadTexture("sun.jpg"); 

        mercury = std::make_unique<Sphere>(mercury_radius, 50, 50);
        mercurytexture = mercury->loadTexture("mercury.jpg");

        venus = std::make_unique<Sphere>(venus_radius, 50, 50);
        venustexture = venus->loadTexture("venus.jpg");

        earth = std::make_unique<Sphere>(earth_radius, 50, 50);
        earthtexture = earth->loadTexture("earth.jpg");

        mars = std::make_unique<Sphere>(mars_radius, 50, 50);
        marstexture = mars->loadTexture("mars.jpg");

        jupiter = std::make_unique<Sphere>(jupiter_radius, 50, 50);
        jupitertexture = jupiter->loadTexture("jupiter.jpg");

        saturn = std::make_unique<Sphere>(saturn_radius, 50, 50);
        saturntexture = saturn->loadTexture("saturn.jpg");

        uranus = std::make_unique<Sphere>(uranus_radius, 50, 50);
        uranustexture = uranus->loadTexture("uranus.jpg");

        neptune = std::make_unique<Sphere>(neptune_radius, 50, 50);
        neptunetexture = neptune->loadTexture("neptune.jpg");

        moon = std::make_unique<Sphere>(moon_radius, 50, 50);
        moontexture = moon->loadTexture("moon.jpg");

        std::cout << "Planet vertex buffers and textures initialized." << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "Error loading planet texture: " << e.what() << std::endl;
        throw;
    }
}

void initExplosionComponents() {
     try {
        particleSystem = std::make_unique<ParticleSystem>(PARTICLE_TEXTURE_PATH, "particle_vs.glsl", "particle_fs.glsl");
        coronaEffect = std::make_unique<CoronaEffect>(CORONA_TEXTURE_PATH, "corona_vs.glsl", "corona_fs.glsl");
        std::cout << "Explosion simulation components loaded successfully." << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "Error initializing explosion simulation components: " << e.what() << std::endl;
        throw;
    }
}


void initOrbitGraphics() {
    std::vector<GLuint> shaderList;
    shaderList.push_back(glFramework::LoadShaderGL(GL_VERTEX_SHADER, "orbit_vs.glsl"));
    shaderList.push_back(glFramework::LoadShaderGL(GL_FRAGMENT_SHADER, "orbit_fs.glsl"));
    orbitShaderProgram = glFramework::CreateProgramGL(shaderList);
    orbitMVPLoc = glGetUniformLocation(orbitShaderProgram, "u_mvp");
    orbitColorLoc = glGetUniformLocation(orbitShaderProgram, "u_orbitColor");

    // lambda for the vao/vbo for orbit path
    auto createOrbitBuffers = [&](OrbitData& data, float a, float b) {
        std::vector<glm::vec3> vertices = generateEllipseVertices(a, b);
        data.vertexCount = vertices.size();
        if (data.vertexCount == 0) return;

        glGenVertexArrays(1, &data.vao);
        glGenBuffers(1, &data.vbo);

        glBindVertexArray(data.vao);
        glBindBuffer(GL_ARRAY_BUFFER, data.vbo);
        glBufferData(GL_ARRAY_BUFFER, data.vertexCount * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    };

    createOrbitBuffers(mercuryOrbit, merc_a, merc_b);
    createOrbitBuffers(venusOrbit, ven_a, ven_b);
    createOrbitBuffers(earthOrbit, earth_a, earth_b);
    createOrbitBuffers(marsOrbit, mars_a, mars_b);
    createOrbitBuffers(jupiterOrbit, jup_a, jup_b);
    createOrbitBuffers(saturnOrbit, sat_a, sat_b);
    createOrbitBuffers(uranusOrbit, uran_a, uran_b);
    createOrbitBuffers(neptuneOrbit, nept_a, nept_b);

    std::cout << "Orbit graphics initialized." << std::endl;
}


void updateSimulation(float currentTime, float deltaTime) {
    const glm::vec3 explosionCenter(0.0f);


    if (currentState == SimulationState::STATE_CORONA_EXPANDING && coronaStartTime > 0.0f && currentTime >= coronaStartTime + CORONA_TO_PARTICLE_DELAY) {
        currentState = SimulationState::STATE_EXPLODING;
        explosionStartTime = currentTime; 
        std::cout << "STATE: EXPLODING (particle burst) at time " << currentTime << std::endl;
        if (particleSystem) {
            particleSystem->spawnInitialBurst(explosionCenter);
        }
    }
    else if (currentState == SimulationState::STATE_EXPLODING && explosionStartTime > 0.0f && currentTime >= explosionStartTime + EXPLOSION_TRANSITION_DURATION) {
        currentState = SimulationState::STATE_FADING;
        std::cout << "STATE: FADING at time " << currentTime << std::endl;
    }

    if (currentState >= SimulationState::STATE_CORONA_EXPANDING && 
        coronaStartTime > 0.0f && 
        !planetsEscapeTriggered && 
        currentTime >= coronaStartTime + PLANET_ESCAPE_DELAY)
    {
        planetsEscapeTriggered = true; 
        std::cout << "PLANET ESCAPE triggered at time " << currentTime << std::endl;

        // only earth, moon, mars, jupiter, saturn, uranus, neptune would escape
        // mercury and venus would get boiled (as per our assumption)

        // earth escape triggering
        earthStartPos = glm::vec3(earth_a * cos(glm::radians(earthRevolutionAngle)), 0.0f, earth_b * sin(glm::radians(earthRevolutionAngle)));
        earthEscaping = true;
        earthEscapeTime = 0.0f;

        // for the moon also
        glm::vec3 currentEarthCenter = earthStartPos;
        float moonRelX = moon_orbit_radius * cos(glm::radians(moonRevolutionAngle));
        float moonRelZ = moon_orbit_radius * sin(glm::radians(moonRevolutionAngle));

        moonStartPos = currentEarthCenter + glm::vec3(moonRelX, 0.0f, moonRelZ);
        moonEscaping = true;
        moonEscapeTime = 0.0f;

        // mars escape
        marsStartPos = glm::vec3(mars_a * cos(glm::radians(marsRevolutionAngle)), 0.0f, mars_b * sin(glm::radians(marsRevolutionAngle)));
        marsEscaping = true;
        marsEscapeTime = 0.0f;

        // jupiter escape
        jupiterStartPos = glm::vec3(jup_a * cos(glm::radians(jupiterRevolutionAngle)), 0.0f, jup_b * sin(glm::radians(jupiterRevolutionAngle)));
        jupiterEscaping = true;
        jupiterEscapeTime = 0.0f;

        // saturn escape
        saturnStartPos = glm::vec3(sat_a * cos(glm::radians(saturnRevolutionAngle)), 0.0f, sat_b * sin(glm::radians(saturnRevolutionAngle)));
        saturnEscaping = true;
        saturnEscapeTime = 0.0f;

        // uranus escape
        uranusStartPos = glm::vec3(uran_a * cos(glm::radians(uranusRevolutionAngle)), 0.0f, uran_b * sin(glm::radians(uranusRevolutionAngle)));
        uranusEscaping = true;
        uranusEscapeTime = 0.0f;

        // neptune escape
        neptuneStartPos = glm::vec3(nept_a * cos(glm::radians(neptuneRevolutionAngle)), 0.0f, nept_b * sin(glm::radians(neptuneRevolutionAngle)));
        neptuneEscaping = true;
        neptuneEscapeTime = 0.0f;

    }


    // explosion
    if (coronaEffect) {
        coronaEffect->update(deltaTime, currentTime, currentState, coronaStartTime);
    }
    if (particleSystem && (currentState == SimulationState::STATE_EXPLODING || currentState == SimulationState::STATE_FADING)) {
        particleSystem->update(deltaTime, currentTime, explosionCenter);
    }


    sunRotationAngle += 0.5f * deltaTime * 60.0f; 


    switch (currentState) {
        case SimulationState::STATE_PRE_EXPLOSION:
            currentSunScaledRadius = sun_radius; // at the intiial size 
            break;
        case SimulationState::STATE_CORONA_EXPANDING:
            if (coronaStartTime > 0.0f) { // sun grows larger to a size slightly lesser than corona, so that it is visible
                float timeSinceTrigger = currentTime - coronaStartTime;
                float expansionProgress = glm::clamp(timeSinceTrigger / CORONA_TO_PARTICLE_DELAY, 0.0f, 1.0f);
                float expansionCurve = 1.0f - pow(1.0f - expansionProgress, 3.0f);
                currentSunScaledRadius = glm::mix(sun_radius, CORONA_BASE_SIZE - 1.0f, expansionCurve);
            } else {
                currentSunScaledRadius = sun_radius;
            }
            break;
        case SimulationState::STATE_EXPLODING:
        case SimulationState::STATE_FADING:
             // but at this phase sun wouldnt be rendered
             if (coronaStartTime > 0.0f) {
                 currentSunScaledRadius = CORONA_BASE_SIZE -1.0f;
             } else {
                 currentSunScaledRadius = sun_radius;
             }

            break;
    }

    // only earth, moon and other planets except mercury and venus escape
    // we can show that mercury and venus would get engulfed in the supernova

    mercuryRevolutionAngle += 0.75f * deltaTime * 60.0f;
    mercuryRotationAngle += 1.0f * deltaTime * 60.0f;

    venusRevolutionAngle += 0.55f * deltaTime * 60.0f;
    venusRotationAngle += 1.0f * deltaTime * 60.0f;


   
    if (earthEscaping) {
        earthEscapeTime += PLANET_ESCAPE_SPEED_MULTIPLIER * deltaTime;
        earthEscapeTime = glm::clamp(earthEscapeTime, 0.0f, 1.0f);
        earthRotationAngle += 1.0f * deltaTime * 60.0f; 
    } else {
        earthRevolutionAngle += 0.45f * deltaTime * 60.0f;
        earthRotationAngle += 1.0f * deltaTime * 60.0f;
    }

    // Moon
    if (moonEscaping) {
        moonEscapeTime += PLANET_ESCAPE_SPEED_MULTIPLIER * deltaTime;
        moonEscapeTime = glm::clamp(moonEscapeTime, 0.0f, 1.0f);
        moonRotationAngle += 1.5f * deltaTime * 60.0f; 
    } else {
        moonRevolutionAngle += 1.5f * deltaTime * 60.0f; 
        moonRotationAngle += 1.0f * deltaTime * 60.0f;
    }

    if (marsEscaping) {
        marsEscapeTime += PLANET_ESCAPE_SPEED_MULTIPLIER * deltaTime;
        marsEscapeTime = glm::clamp(marsEscapeTime, 0.0f, 1.0f);
        marsRotationAngle += 1.0f * deltaTime * 60.0f;
    } else {
        marsRevolutionAngle += 0.35f * deltaTime * 60.0f;
        marsRotationAngle += 1.0f * deltaTime * 60.0f;
    }

    if (jupiterEscaping) {
        jupiterEscapeTime += PLANET_ESCAPE_SPEED_MULTIPLIER * deltaTime;
        jupiterEscapeTime = glm::clamp(jupiterEscapeTime, 0.0f, 1.0f);
        jupiterRotationAngle += 1.0f * deltaTime * 60.0f;
    } else {
        jupiterRevolutionAngle += 0.25f * deltaTime * 60.0f;
        jupiterRotationAngle += 1.0f * deltaTime * 60.0f;
    }

    if (saturnEscaping) {
        saturnEscapeTime += PLANET_ESCAPE_SPEED_MULTIPLIER * deltaTime;
        saturnEscapeTime = glm::clamp(saturnEscapeTime, 0.0f, 1.0f);
        saturnRotationAngle += 1.0f * deltaTime * 60.0f;
    } else {
        saturnRevolutionAngle += 0.2f * deltaTime * 60.0f;
        saturnRotationAngle += 1.0f * deltaTime * 60.0f;
    }

    if (uranusEscaping) {
        uranusEscapeTime += PLANET_ESCAPE_SPEED_MULTIPLIER * deltaTime;
        uranusEscapeTime = glm::clamp(uranusEscapeTime, 0.0f, 1.0f);
        uranusRotationAngle += 1.0f * deltaTime * 60.0f;
    } else {
        uranusRevolutionAngle += 0.15f * deltaTime * 60.0f;
        uranusRotationAngle += 1.0f * deltaTime * 60.0f;
    }

    if (neptuneEscaping) {
        neptuneEscapeTime += PLANET_ESCAPE_SPEED_MULTIPLIER * deltaTime;
        neptuneEscapeTime = glm::clamp(neptuneEscapeTime, 0.0f, 1.0f);
        neptuneRotationAngle += 1.0f * deltaTime * 60.0f;
    } else {
        neptuneRevolutionAngle += 0.1f * deltaTime * 60.0f;
        neptuneRotationAngle += 1.0f * deltaTime * 60.0f;
    }

    if (explosionStartTime > 0.0f) { // if explosion has started, after the respective delays 
        // the rendering of mercury and venus is stopped
        float currentTime = static_cast<float>(glfwGetTime());
        if (mercuryRender && currentTime >= explosionStartTime + MERCURY_DISAPPEAR_DELAY) {
            mercuryRender = false;
            std::cout << "Mercury rendering stopped at time " << currentTime << std::endl;
        }
        if (venusRender && currentTime >= explosionStartTime + VENUS_DISAPPEAR_DELAY) {
            venusRender = false;
             std::cout << "Venus rendering stopped at time " << currentTime << std::endl;
        }
    }

    if (isPathAnimationDone && !fadingOrbits) {
        fadingOrbits = true; 
        orbitFadeTimer = 0.0f;
        std::cout << "Orbit fade starting." << std::endl;
    }

 
    if (fadingOrbits) {
        orbitFadeTimer += deltaTime;
        orbitAlpha = glm::max(0.0f, 1.0f - (orbitFadeTimer / orbitFadeDuration));
    } else if (!isPathAnimationDone) {
        orbitAlpha = 1.0f;
        fadingOrbits = false;
    }

    // for the camera zoom
     if (!finalZoomKeyframeAdded && currentState >= SimulationState::STATE_CORONA_EXPANDING && coronaStartTime > 0.0f) {
        float timeSinceS = currentTime - coronaStartTime;
        if (timeSinceS >= finalZoomDelay) {
            finalZoomKeyframeAdded = true;
            // this is for the final camera position
            cameraPath.push_back({ finalZoomTargetPos, finalZoomLookAt, finalZoomDuration });
            std::cout << "CAMERA ZOOM: Added final zoom-in keyframe to path (Target: "
                      << finalZoomTargetPos.x << "," << finalZoomTargetPos.y << "," << finalZoomTargetPos.z
                      << " over " << finalZoomDuration << "s)" << std::endl;

 
            if (isPathAnimationDone) {
                 isPathAnimationDone = false;
                 if (cameraPath.size() >= 2) {
                    currentPathSegment = cameraPath.size() - 2;
                 } else {
                    currentPathSegment = 0;
                 }
                 segmentElapsedTime = 0.0f;
            }
        }
    }

}



void renderGL(GLFWwindow* window, const glm::mat4& view, const glm::mat4& projection) {

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 vp = projection * view;
    glm::mat4 orbitModelMatrix = glm::mat4(1.0f);


    glm::mat4 orbitMVP = vp * orbitModelMatrix; 

    if (orbitAlpha > 0.001f) {
        glUseProgram(orbitShaderProgram);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE); 
        glEnable(GL_DEPTH_TEST);

        glUniformMatrix4fv(orbitMVPLoc, 1, GL_FALSE, glm::value_ptr(orbitMVP));
        glUniform4f(orbitColorLoc, 0.6f, 0.6f, 0.6f, orbitAlpha); //grey color


        auto drawOrbit = [&](const OrbitData& data) {
            if (data.vao != 0 && data.vertexCount > 0) {
                glBindVertexArray(data.vao);
                glDrawArrays(GL_LINE_LOOP, 0, data.vertexCount);
            }
        };

        drawOrbit(mercuryOrbit);
        drawOrbit(venusOrbit);
        drawOrbit(earthOrbit);
        drawOrbit(marsOrbit);
        drawOrbit(jupiterOrbit);
        drawOrbit(saturnOrbit);
        drawOrbit(uranusOrbit);
        drawOrbit(neptuneOrbit);

        glBindVertexArray(0); 
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }
    // rendering of orits done, now planets
    { 
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glEnable(GL_CULL_FACE);
        glDisable(GL_BLEND);

        glUseProgram(planetShaderProgram);

        GLint modelLoc = glGetUniformLocation(planetShaderProgram, "model");
        GLint viewLoc = glGetUniformLocation(planetShaderProgram, "view");
        GLint projectionLoc = glGetUniformLocation(planetShaderProgram, "projection");
        GLint lightPosLoc = glGetUniformLocation(planetShaderProgram, "lightPos");
        GLint viewPosLoc = glGetUniformLocation(planetShaderProgram, "viewPos"); 
        GLint lightColorLoc = glGetUniformLocation(planetShaderProgram, "lightColor");
        GLint isSunLoc = glGetUniformLocation(planetShaderProgram, "isSun");
        GLint isMercLoc = glGetUniformLocation(planetShaderProgram, "isMerc");
        GLint timeLoc = glGetUniformLocation(planetShaderProgram, "u_time");


        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3f(lightPosLoc, 0.0f, 0.0f, 0.0f); // light source
        glm::vec3 cameraPos = glm::vec3(glm::inverse(view)[3]); 
        glUniform3fv(viewPosLoc, 1, glm::value_ptr(cameraPos));
        glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);
        if (timeLoc != -1) glUniform1f(timeLoc, (float)glfwGetTime());
        if (isMercLoc != -1) glUniform1i(isMercLoc, 0);


        // sun is rendered only before explosion or during corona expansion (scales up)
        if (currentState == SimulationState::STATE_PRE_EXPLOSION || currentState == SimulationState::STATE_CORONA_EXPANDING) {
            if (currentSunScaledRadius > 0.01f) {
                 glm::mat4 sun_model = glm::mat4(1.0f);
                 sun_model = glm::rotate(sun_model, glm::radians(sunRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
                 sun_model = glm::scale(sun_model, glm::vec3(currentSunScaledRadius)); 

                 if (isSunLoc != -1) glUniform1i(isSunLoc, 1);
                 if (Sun) Sun->draw(modelLoc, sun_model, suntexture);
                 if (isSunLoc != -1) glUniform1i(isSunLoc, 0); 
            }
        }


        if (mercuryRender && mercury) {
            glm::mat4 merc_model = glm::mat4(1.0f);
            glm::vec3 mercuryCurrentPos;
            float xmerc = merc_a * cos(glm::radians(mercuryRevolutionAngle));
            float zmerc = merc_b * sin(glm::radians(mercuryRevolutionAngle));
            merc_model = glm::translate(merc_model, glm::vec3(xmerc, 0.0f, zmerc));
            merc_model = glm::rotate(merc_model, glm::radians(mercuryRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
            merc_model = glm::scale(merc_model, glm::vec3(mercury_radius));
            mercury->draw(modelLoc, merc_model, mercurytexture);
        }

         if (venusRender && venus) {
            glm::mat4 ven_model = glm::mat4(1.0f);
            glm::vec3 venusCurrentPos;
            float xvenus = ven_a * cos(glm::radians(venusRevolutionAngle));
            float zvenus = ven_b * sin(glm::radians(venusRevolutionAngle));
            ven_model = glm::translate(ven_model, glm::vec3(xvenus, 0.0f, zvenus));
            ven_model = glm::rotate(ven_model, glm::radians(venusRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
            ven_model = glm::scale(ven_model, glm::vec3(venus_radius));
            venus->draw(modelLoc, ven_model, venustexture);
        }


        glm::vec3 earthCurrentPos;
        if (earth) {
            glm::mat4 earth_model = glm::mat4(1.0f);
            if (earthEscaping) {
                // bezier curve
                glm::vec3 P0 = earthStartPos;
                glm::vec3 dir_out = (length(P0) > 0.01f) ? normalize(P0) : glm::vec3(1,0,0);
                glm::vec3 dir_perp = normalize(cross(dir_out, glm::vec3(0,1,0)));
                 if (length(dir_perp) < 0.1f) dir_perp = normalize(cross(dir_out, glm::vec3(1,0,0)));

                glm::vec3 P1 = P0 + dir_out * 25.0f + dir_perp * 12.0f;
                glm::vec3 P2 = P0 + dir_out * 55.0f - dir_perp * 18.0f;
                glm::vec3 P3 = P0 + dir_out * 80.0f;

                earthCurrentPos = calculateCubicBezier(earthEscapeTime, P0, P1, P2, P3); // Still need current pos for moon's relative start if needed, though moon escapes too
                earth_model = glm::translate(earth_model, earthCurrentPos);
            } else {
                // the normal orbital trajectory
                float xearth = earth_a * cos(glm::radians(earthRevolutionAngle));
                float zearth = earth_b * sin(glm::radians(earthRevolutionAngle));
                earthCurrentPos = glm::vec3(xearth, 0.0f, zearth);
                earth_model = glm::translate(earth_model, earthCurrentPos);
            }
            earth_model = glm::rotate(earth_model, glm::radians(earthRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
            earth_model = glm::scale(earth_model, glm::vec3(earth_radius));
            earth_model = glm::rotate(earth_model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate to face the camera
            earth->draw(modelLoc, earth_model, earthtexture);
        }



        if (moon) {
             glm::mat4 moon_model = glm::mat4(1.0f);
             glm::vec3 moonCurrentPos;
             if (moonEscaping) {
                glm::vec3 P0 = moonStartPos; // strat of escape trigger
                glm::vec3 dir_out = (length(P0) > 0.01f) ? normalize(P0) : glm::vec3(1,0,0);

                glm::vec3 dir_perp = normalize(cross(dir_out, glm::vec3(0.2f, 0.9f, 0.1f)));
                if (length(dir_perp) < 0.1f) dir_perp = normalize(cross(dir_out, glm::vec3(0,0,1)));

                glm::vec3 P1 = P0 + dir_out * 22.0f - dir_perp * 15.0f; // different curve than earth
                glm::vec3 P2 = P0 + dir_out * 50.0f + dir_perp * 10.0f;
                glm::vec3 P3 = P0 + dir_out * 75.0f;

                moonCurrentPos = calculateCubicBezier(moonEscapeTime, P0, P1, P2, P3);
                moon_model = glm::translate(moon_model, moonCurrentPos);
             } else {
                // orbital trajectory relative to earth
                 float moonRelX = moon_orbit_radius * cos(glm::radians(moonRevolutionAngle));
                 float moonRelZ = moon_orbit_radius * sin(glm::radians(moonRevolutionAngle));
                 moonCurrentPos = earthCurrentPos + glm::vec3(moonRelX, 0.0f, moonRelZ);
                 moon_model = glm::translate(moon_model, moonCurrentPos);
             }
             moon_model = glm::rotate(moon_model, glm::radians(moonRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
             moon_model = glm::scale(moon_model, glm::vec3(moon_radius));
             moon->draw(modelLoc, moon_model, moontexture);
        }


        if (mars) {
            glm::mat4 mars_model = glm::mat4(1.0f);
            if (marsEscaping) {
                // bezier curve
                glm::vec3 P0 = marsStartPos;
                glm::vec3 dir_out = (length(P0) > 0.01f) ? normalize(P0) : glm::vec3(1,0,0);
                glm::vec3 dir_perp = normalize(cross(dir_out, glm::vec3(0,1,0)));
                 if (length(dir_perp) < 0.1f) dir_perp = normalize(cross(dir_out, glm::vec3(1,0,0)));

                glm::vec3 P1 = P0 + dir_out * 30.0f + dir_perp * 10.0f;
                glm::vec3 P2 = P0 + dir_out * 60.0f + dir_perp * 5.0f;
                glm::vec3 P3 = P0 + dir_out * 90.0f;

                mars_model = glm::translate(mars_model, calculateCubicBezier(marsEscapeTime, P0, P1, P2, P3));
            } else {
                // orbital trajectory 
                float xmars = mars_a * cos(glm::radians(marsRevolutionAngle));
                float zmars = mars_b * sin(glm::radians(marsRevolutionAngle));
                mars_model = glm::translate(mars_model, glm::vec3(xmars, 0.0f, zmars));
            }
            mars_model = glm::rotate(mars_model, glm::radians(marsRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
            mars_model = glm::scale(mars_model, glm::vec3(mars_radius));
            mars_model = glm::rotate(mars_model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate to face the camera
            mars->draw(modelLoc, mars_model, marstexture);
        }



        if (jupiter) {
            glm::mat4 jup_model = glm::mat4(1.0f);

            if (jupiterEscaping) {

                 glm::vec3 P0 = jupiterStartPos;
                 glm::vec3 dir_out = (length(P0) > 0.01f) ? normalize(P0) : glm::vec3(1,0,0);
                 glm::vec3 dir_perp = normalize(cross(dir_out, glm::vec3(0,1,0)));
                 if (length(dir_perp) < 0.1f) dir_perp = normalize(cross(dir_out, glm::vec3(0,0,1)));

                 glm::vec3 P1 = P0 + dir_out * 40.0f - dir_perp * 20.0f;
                 glm::vec3 P2 = P0 + dir_out * 70.0f - dir_perp * 10.0f;
                 glm::vec3 P3 = P0 + dir_out * 100.0f;

                jup_model = glm::translate(jup_model, calculateCubicBezier(jupiterEscapeTime, P0, P1, P2, P3));
            } else {
                // orbital trajectory 
                 float xjup = jup_a * cos(glm::radians(jupiterRevolutionAngle));
                 float zjup = jup_b * sin(glm::radians(jupiterRevolutionAngle));
                 jup_model = glm::translate(jup_model, glm::vec3(xjup, 0.0f, zjup));
            }
            jup_model = glm::rotate(jup_model, glm::radians(jupiterRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
            jup_model = glm::scale(jup_model, glm::vec3(jupiter_radius));
            jup_model = glm::rotate(jup_model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); 
            jupiter->draw(modelLoc, jup_model, jupitertexture);
        }

        if (saturn) {
             glm::mat4 sat_model = glm::mat4(1.0f);
             if (saturnEscaping) {
                 glm::vec3 P0 = saturnStartPos;
                 glm::vec3 dir_out = (length(P0) > 0.01f) ? normalize(P0) : glm::vec3(1,0,0);
                 glm::vec3 dir_perp = normalize(cross(dir_out, glm::vec3(0,1,0)));
                 if (length(dir_perp) < 0.1f) dir_perp = normalize(cross(dir_out, glm::vec3(1,0,0)));

                 glm::vec3 P1 = P0 + dir_out * 45.0f + dir_perp * 25.0f;
                 glm::vec3 P2 = P0 + dir_out * 75.0f + dir_perp * 15.0f;
                 glm::vec3 P3 = P0 + dir_out * 110.0f;
                 sat_model = glm::translate(sat_model, calculateCubicBezier(saturnEscapeTime, P0, P1, P2, P3));
             } else {
                 // orbital trajectory 
                 float xsat = sat_a * cos(glm::radians(saturnRevolutionAngle));
                 float zsat = sat_b * sin(glm::radians(saturnRevolutionAngle));
                 sat_model = glm::translate(sat_model, glm::vec3(xsat, 0.0f, zsat));
             }
             sat_model = glm::rotate(sat_model, glm::radians(saturnRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
             sat_model = glm::scale(sat_model, glm::vec3(saturn_radius));
             sat_model = glm::rotate(sat_model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); 
             saturn->draw(modelLoc, sat_model, saturntexture);
        }


        if (uranus) {
            glm::mat4 uran_model = glm::mat4(1.0f);

             if (uranusEscaping) {
                 glm::vec3 P0 = uranusStartPos;
                 glm::vec3 dir_out = (length(P0) > 0.01f) ? normalize(P0) : glm::vec3(1,0,0);
                 glm::vec3 dir_perp = normalize(cross(dir_out, glm::vec3(0,1,0)));
                 if (length(dir_perp) < 0.1f) dir_perp = normalize(cross(dir_out, glm::vec3(0,0,1)));

                 glm::vec3 P1 = P0 + dir_out * 50.0f - dir_perp * 10.0f;
                 glm::vec3 P2 = P0 + dir_out * 80.0f - dir_perp * 20.0f;
                 glm::vec3 P3 = P0 + dir_out * 120.0f;
                 uran_model = glm::translate(uran_model, calculateCubicBezier(uranusEscapeTime, P0, P1, P2, P3));
             } else {
                // orbital trajectory 
                 float xura = uran_a * cos(glm::radians(uranusRevolutionAngle));
                 float zura = uran_b * sin(glm::radians(uranusRevolutionAngle));
                 uran_model = glm::translate(uran_model, glm::vec3(xura, 0.0f, zura));
             }
            uran_model = glm::rotate(uran_model, glm::radians(uranusRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
            uran_model = glm::scale(uran_model, glm::vec3(uranus_radius));
            uran_model = glm::rotate(uran_model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); 
            uranus->draw(modelLoc, uran_model, uranustexture);
        }

        if (neptune) {
            glm::mat4 nept_model = glm::mat4(1.0f);
             if (neptuneEscaping) {
                 glm::vec3 P0 = neptuneStartPos;
                 glm::vec3 dir_out = (length(P0) > 0.01f) ? normalize(P0) : glm::vec3(1,0,0);
                 glm::vec3 dir_perp = normalize(cross(dir_out, glm::vec3(0,1,0)));
                 if (length(dir_perp) < 0.1f) dir_perp = normalize(cross(dir_out, glm::vec3(1,0,0)));

                 glm::vec3 P1 = P0 + dir_out * 55.0f + dir_perp * 5.0f;
                 glm::vec3 P2 = P0 + dir_out * 85.0f - dir_perp * 8.0f;
                 glm::vec3 P3 = P0 + dir_out * 130.0f;
                 nept_model = glm::translate(nept_model, calculateCubicBezier(neptuneEscapeTime, P0, P1, P2, P3));
             } else {
                 // orbital trajectory 
                 float xnept = nept_a * cos(glm::radians(neptuneRevolutionAngle));
                 float znept = nept_b * sin(glm::radians(neptuneRevolutionAngle));
                 nept_model = glm::translate(nept_model, glm::vec3(xnept, 0.0f, znept));
             }
            nept_model = glm::rotate(nept_model, glm::radians(neptuneRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
            nept_model = glm::scale(nept_model, glm::vec3(neptune_radius));
            nept_model = glm::rotate(nept_model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); 
            neptune->draw(modelLoc, nept_model, neptunetexture);
        }


    } 

   // explosion
    { 
        const glm::vec3 explosionCenter(0.0f);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE); // used for additive blending
        glDepthMask(GL_FALSE);
        glDisable(GL_CULL_FACE); // since its billboards and particles
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);


        // corona is rendered only if its expanding state
        if (coronaEffect && currentState >= SimulationState::STATE_CORONA_EXPANDING) {
             coronaEffect->render(view, projection, explosionCenter);
        }

        // particles only if expanding state
        if (particleSystem && currentState >= SimulationState::STATE_EXPLODING) {
            particleSystem->render(view, projection);
        }

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glEnable(GL_CULL_FACE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
        glDepthFunc(GL_LESS); 

    } 
}


int main() {
    glfwSetErrorCallback(glFramework::error_callback);
    if (!glfwInit()) {
         std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Supernova Simulation", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        std::cerr << "GLEW Init Failed: " << glewGetErrorString(err) << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    while(glGetError() != GL_NO_ERROR);
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    glfwSetFramebufferSizeCallback(window, glFramework::framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);

    
    try {
        initPlanetShadersGL();
        initPlanetVertexBuffers();
        initExplosionComponents();
        initOrbitGraphics(); 
        initializeCameraPath();
    } catch (const std::runtime_error& e) {
         std::cerr << "Initialization failed: " << e.what() << std::endl;
         particleSystem.reset();
         coronaEffect.reset();
         Sun.reset(); mercury.reset(); venus.reset(); earth.reset();
         moon.reset(); mars.reset(); jupiter.reset(); saturn.reset(); uranus.reset(); neptune.reset();
         glfwDestroyWindow(window);
         glfwTerminate();
         return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_PROGRAM_POINT_SIZE);

    lastFrameTime = static_cast<float>(glfwGetTime()); // Initial value
    float fpsTime = 0.0f;
    int frameCount = 0;

    // Store the previous pause state to detect changes
    bool wasPausedLastFrame = isPaused;

    while (!glfwWindowShouldClose(window)) {

        float currentTime = static_cast<float>(glfwGetTime());
        float actualDeltaTime = currentTime - lastFrameTime; // Real 
        if (wasPausedLastFrame && !isPaused) {
            lastFrameTime = currentTime; // Prevents large jump by restarting the timer
            actualDeltaTime = 0.0f;      // Ensure delta time is 0 for the first frame after unpause
             std::cout << "Resetting lastFrameTime after unpause." << std::endl;
        }
        wasPausedLastFrame = isPaused; // Update for next frame check

        // Use 0 delta time if paused, otherwise use the actual elapsed time
        float deltaTime = isPaused ? 0.0f : actualDeltaTime;

        // Update lastFrameTime ONLY if not paused, so it doesn't advance during pause
        if (!isPaused) {
             lastFrameTime = currentTime;
        }

        deltaTime = glm::clamp(deltaTime, 0.0f, 0.1f);

        frameCount++;
        fpsTime += deltaTime;
        if (fpsTime >= 1.0f) {
            std::stringstream ss;
            ss << "Supernova Simulation | FPS: " << frameCount << (isPaused ? " (PAUSED)" : "");
            glfwSetWindowTitle(window, ss.str().c_str());
            frameCount = 0;
            fpsTime -= 1.0f;
        }


        processInput(window, deltaTime);


        updateSimulation(currentTime, deltaTime);


        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        float aspectRatio = (display_h > 0) ? (float)display_w / (float)display_h : 1.0f;
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 300.0f); // Increased far plane further

     
    glm::mat4 view;
    glm::vec3 baseCamPos; 
    glm::vec3 baseLookAt; 

    if (!isPathAnimationDone && !cameraPath.empty()) {
        // Using deltaTime which will be 0 if paused
       segmentElapsedTime += deltaTime;
       // ... rest of camera path logic ...
        if (currentPathSegment + 1 < cameraPath.size()) {
            const CameraKeyframe& startKeyframe = cameraPath[currentPathSegment];
            const CameraKeyframe& endKeyframe = cameraPath[currentPathSegment + 1];
            float segmentDuration = endKeyframe.durationToReach;
            float t = (segmentDuration > 0.0001f) ? glm::clamp(segmentElapsedTime / segmentDuration, 0.0f, 1.0f) : 1.0f;
            float smooth_t = glm::smoothstep(0.0f, 1.0f, t);
            baseCamPos = glm::mix(startKeyframe.position, endKeyframe.position, smooth_t);
            baseLookAt = glm::mix(startKeyframe.lookAt, endKeyframe.lookAt, smooth_t);
            if (segmentElapsedTime >= segmentDuration && segmentDuration > 0.0f) { // Check duration > 0 to prevent infinite loop if paused exactly at segment end
                currentPathSegment++;
                segmentElapsedTime = 0.0f;
                if (currentPathSegment >= cameraPath.size() - 1) {
                    isPathAnimationDone = true;
                    baseCamPos = cameraPath.back().position;
                    baseLookAt = cameraPath.back().lookAt;
                    std::cout << "Camera path animation finished." << std::endl;
                }
            }
        } else { // Should only reach here if cameraPath has 0 or 1 element, or after last segment
            isPathAnimationDone = true;
             if (!cameraPath.empty()) {
                baseCamPos = cameraPath.back().position;
                baseLookAt = cameraPath.back().lookAt;
            }
        }
   }

   if (isPathAnimationDone || cameraPath.empty()) {
    if (!cameraPath.empty()) {
       baseCamPos = cameraPath.back().position;
       baseLookAt = cameraPath.back().lookAt;
    } else {
       baseCamPos = glm::vec3(0.0f, 35.0f, 60.0f);
       baseLookAt = glm::vec3(0.0f);
    }
}

   
glm::vec3 finalCamPos = baseCamPos;
view = glm::lookAt(finalCamPos, baseLookAt, glm::vec3(0.0f, 1.0f, 0.0f));

// --- Render Frame ---
renderGL(window, view, projection);

// --- Swap Buffers & Poll Events ---
glfwSwapBuffers(window);
glfwPollEvents(); // Event polling should always happen
}


    std::cout << "Cleaning up..." << std::endl;
    particleSystem.reset();
    coronaEffect.reset();
    auto deleteOrbitData = [](OrbitData& data) {
        glDeleteVertexArrays(1, &data.vao);
        glDeleteBuffers(1, &data.vbo);
        data.vao = 0; data.vbo = 0; data.vertexCount = 0;
    };
    deleteOrbitData(mercuryOrbit);
    deleteOrbitData(venusOrbit);
    deleteOrbitData(earthOrbit);
    deleteOrbitData(marsOrbit);
    deleteOrbitData(jupiterOrbit);
    deleteOrbitData(saturnOrbit);
    deleteOrbitData(uranusOrbit);
    deleteOrbitData(neptuneOrbit);
    glDeleteProgram(orbitShaderProgram);

    glDeleteProgram(planetShaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();
    std::cout << "Exiting." << std::endl;
    return 0;
}


void initializeCameraPath() {
    cameraPath.clear();
    const glm::vec3 centerTarget(0.0f, 0.0f, 0.0f);

    // the 0th keyframe, intial pos
    cameraPath.push_back({glm::vec3(0.0f, 25.0f, 70.0f), centerTarget, 0.0f}); 

    // 1st keyframe
    cameraPath.push_back({glm::vec3(40.0f, 20.0f, 65.0f), centerTarget, 6.0f}); // 6.0f --> time for the transition

    // the next keyframe ...
    cameraPath.push_back({glm::vec3(30.0f, 40.0f, 40.0f), centerTarget, 8.0f});

    cameraPath.push_back({glm::vec3(10.0f, 50.0f, 50.0f), centerTarget, 7.0f}); 


    cameraPath.push_back({glm::vec3(30.0f, 30.0f, 0.0f), centerTarget, 6.0f}); 

    cameraPath.push_back({glm::vec3(10.0f, 60.0f, 50.0f), centerTarget, 5.0f}); 

    isPathAnimationDone = (cameraPath.size() < 2);
    currentPathSegment = 0;
    segmentElapsedTime = 0.0f;

    std::cout << "Camera path initialized with " << cameraPath.size() << " keyframes." << std::endl;
}