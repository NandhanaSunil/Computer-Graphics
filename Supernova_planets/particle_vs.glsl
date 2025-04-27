#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in float aSize;
layout (location = 3) in float aLife;

out vec4 particleColor;
out float particleLife;

uniform mat4 projection;
uniform mat4 view;
uniform float uSizeFadeExponent;
uniform float uBasePointSize;
uniform float u_time; // if time based animation needed but done in cpu

void main()
{
    gl_Position = projection * view * vec4(aPos, 1.0);
    float lifeFactor = pow(aLife, uSizeFadeExponent);
    // point size doesnt become zero..
    gl_PointSize = max(2.0, aSize * lifeFactor * uBasePointSize);

    if (aLife <= 0.0 && uSizeFadeExponent < 0.0) {
         gl_PointSize = 0.0 + 3.0 * sin(u_time + aPos.x * 5.0);; // adding a default minimum
    }


    particleColor = aColor;
    particleLife = aLife;
}