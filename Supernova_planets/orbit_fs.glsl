#version 330 core
out vec4 FragColor;

uniform vec4 u_orbitColor; 

void main()
{
    FragColor = u_orbitColor;
}