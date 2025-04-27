#version 330 core
layout (location = 0) in vec2 aPos; // Simple 2D position for quad vertices

out vec2 TexCoords; // Pass texture coords if needed, or just position

void main()
{
    // Directly output Normalized Device Coordinates (NDC) for a fullscreen quad
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
    // Optionally calculate texture coordinates if the FS needs them based on vertices
    // TexCoords = aPos * 0.5 + 0.5; // Example: map -1..1 to 0..1
}