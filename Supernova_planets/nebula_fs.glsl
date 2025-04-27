#version 330 core

out vec4 FragColor;

uniform vec2 u_resolution;
uniform float u_time;
uniform float u_nebulaExpansionRadius; // Added: Controls visible radius (0 to ~2.0)

// --- Noise functions (hash, noise, fbm) ---
// ... (Keep your noise functions) ...
float hash(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }
float noise(vec2 p) { vec2 i=floor(p),f=fract(p); float a=hash(i),b=hash(i+vec2(1,0)),c=hash(i+vec2(0,1)),d=hash(i+vec2(1,1)); vec2 u=f*f*(3.0-2.0*f); return mix(mix(a,b,u.x),mix(c,d,u.x),u.y); }
float fbm(vec2 p) { float t=0.,a=.5,f=1.; for(int i=0;i<5;++i){t+=noise(p*f)*a;f*=2.;a*=.5;} return t; }
// --- End Noise Functions ---


void main() {
    // Calculate UV coordinates from screen pixel coordinates (-1 to 1, aspect corrected)
    vec2 uv = (gl_FragCoord.xy / u_resolution.xy) * 2.0 - 1.0;
    uv.x *= u_resolution.x / u_resolution.y;

    // --- Expansion Mask ---
    float dist = length(uv); // Distance from center in aspect-corrected uv space
    // Smoothly fade out fragments beyond the current expansion radius
    // Alpha goes from 1 inside radius - edgeWidth to 0 outside radius + edgeWidth
    float edgeWidth = 0.15; // How soft the edge is
    float mask = smoothstep(u_nebulaExpansionRadius + edgeWidth, u_nebulaExpansionRadius - edgeWidth, dist);

    if (mask < 0.01) discard; // Discard fully transparent fragments

    // --- Nebula Calculation (only if visible) ---
    float time = u_time * 0.1;
    vec2 p = uv * 2.0 + vec2(cos(time * 0.15), sin(time * 0.2));
    p += vec2(time * 0.05, -time * 0.03);
    float n = fbm(p);

    // --- Nebula Color Palette ---
    vec3 color_purple = vec3(0.3, 0.1, 0.7);
    vec3 color_pink   = vec3(0.9, 0.2, 0.8);
    vec3 color_blue   = vec3(0.2, 0.6, 1.0);
    vec3 color_dark   = vec3(0.01, 0.01, 0.02);

    vec3 color = mix(color_dark, color_purple, smoothstep(0.0, 0.4, n));
    color = mix(color, color_pink,   smoothstep(0.3, 0.6, n));
    color = mix(color, color_blue,   smoothstep(0.5, 0.8, n));

    // --- Brightness and Glow ---
    float brightness = pow(max(0.0, n), 1.5) * 1.3 + 0.1;
    brightness = clamp(brightness, 0.0, 1.5);

    // Final color: Apply brightness and the expansion mask to alpha
    FragColor = vec4(color * brightness, mask); // Use mask for alpha
}