#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in float aAlpha;
layout (location = 2) in float aSize;

out float Alpha;

uniform mat4 view;
uniform mat4 projection;

void main() {
    Alpha = aAlpha;
    gl_Position = projection * view * vec4(aPos, 1.0);
    gl_PointSize = aSize;
}
