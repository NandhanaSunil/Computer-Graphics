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

void main()
{
    gl_Position = projection * view * vec4(aPos, 1.0);
    float lifeFactor = pow(aLife, uSizeFadeExponent);
    gl_PointSize = max(2.0, aSize * lifeFactor * uBasePointSize);

    particleColor = aColor;
    particleLife = aLife;
}