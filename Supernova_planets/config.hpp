#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <glm/glm.hpp> 
//screen
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// sun particles
const int MAX_PARTICLES = 30000;
// const float PARTICLE_LIFETIME_BASE = 6.0f;
// const float PARTICLE_LIFETIME_LONG = 9.0f;

const float PARTICLE_LIFETIME_BASE = 50.0f;
const float PARTICLE_LIFETIME_LONG = 200.0f; 

const float EXPLOSION_RADIUS = 4.0f; 

const float MAX_INITIAL_SPEED = 1.0; 
const float MIN_INITIAL_SPEED = 0.05f; // Slower start

const float DRAG_FACTOR = 0.15f; 

// const float FADE_EXPONENT = 3.5f; //2.5
// const float SIZE_FADE_EXPONENT = 3.0f;
// const float PARTICLE_BASE_POINT_SIZE = 25.0f; 
const float FADE_EXPONENT = 3.5f;
const float SIZE_FADE_EXPONENT = 3.0f;
const float PARTICLE_BASE_POINT_SIZE = 25.0f;

// texture for corona and particles
const std::string PARTICLE_TEXTURE_PATH = "particle.png";
const std::string CORONA_TEXTURE_PATH = "corona.png";

const float TURBULENCE_STRENGTH = 0.9f;
const float TURBULENCE_FREQUENCY = 1.9f;
const float TURBULENCE_TIME_FACTOR = 0.1f;

const float SECONDARY_PARTICLE_FRACTION = 0.15f;
const float DENSITY_BIAS_FACTOR = 0.3f;
// THESE  attributes needs to be adjusted later.  will do it

const float CORONA_TO_PARTICLE_DELAY = 8.0f; // NEW: Delay before particles burst (in seconds)

// enum class SimulationState { STATE_PRE_EXPLOSION, STATE_EXPLODING, STATE_FADING };

enum class SimulationState {
    STATE_PRE_EXPLOSION,
    STATE_CORONA_EXPANDING, // New state
    STATE_EXPLODING,
    STATE_FADING
};
// const float PRE_EXPLOSION_DURATION = 4.0f;
const float EXPLOSION_TRANSITION_DURATION = 1.5f;

// corona
const int NUM_CORONA_LAYERS = 15;
const float CORONA_BASE_SIZE = 3.5f; //2.5
const float CORONA_PULSE_MAGNITUDE = 0.15f; // currently unused 
const float CORONA_PULSE_SPEED = 3.0f;     // currently unused
const float CORONA_INITIAL_BRIGHTNESS = 2.0f;
const float CORONA_FADE_SPEED = 4.0f;
const float CORONA_LAYER_ROTATION_SPEED = 0.05f;

const float CORONA_ACTUAL_FADE_DURATION = 2.0f;

const float CORONA_MAX_EXPANSION_FACTOR = 2.0f; 

// camera
const glm::vec3 INITIAL_CAMERA_POS = glm::vec3(0.0f, 0.0f, 22.0f);
const glm::vec3 INITIAL_CAMERA_FRONT = glm::vec3(0.0f, 0.0f, -1.0f);
const glm::vec3 WORLD_UP = glm::vec3(0.0f, 1.0f, 0.0f);
const float CAMERA_SPEED = 5.0f;

const float INTRO_SWEEP_DURATION = 9.0f;

#endif // CONFIG_H