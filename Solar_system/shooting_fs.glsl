#version 330 core
in float Alpha;
out vec4 FragColor;

void main() {
    // Yellow-white color with fading alpha
    vec3 color = mix(vec3(1.0, 0.8, 0.2), vec3(1.0, 1.0, 1.0), Alpha);
    FragColor = vec4(color, Alpha);
    
    // Make circular points with smooth edges
    vec2 circCoord = 2.0 * gl_PointCoord - 1.0;
    float falloff = smoothstep(1.0, 0.8, dot(circCoord, circCoord));
    FragColor.a *= falloff;
}