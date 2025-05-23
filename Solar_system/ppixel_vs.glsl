#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 inColor;  // vertex color
layout(location = 3) in vec2 aTexCoord;

uniform int isMoon ;
uniform int isSun ;

out vec3 FragPos;
out vec3 Normal;
out vec3 VertexColor; 
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(position, 1.0)); 
    Normal = normalize(mat3(transpose(inverse(model))) * normal);  

    gl_Position = projection * view * vec4(FragPos, 1.0);
    TexCoord = aTexCoord;
}
