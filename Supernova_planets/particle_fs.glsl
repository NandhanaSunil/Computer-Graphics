#version 330 core
in vec4 particleColor;
in float particleLife;
out vec4 FragColor;

uniform sampler2D particleTexture;
uniform float uFadeExponent;
// uniform float u_time; // if time based effects are needed, but here time baseed are done in cpu

void main()
{
    vec4 texColor = texture(particleTexture, gl_PointCoord);
    // if the transparency is less dont render
    if (texColor.a < 0.02) discard;

    float fadeFactor = (particleLife <= 0.0 && uFadeExponent < 0.0) ? 0.0 : pow(particleLife, uFadeExponent);
    fadeFactor = clamp(fadeFactor, 0.0, 1.0); 

    vec4 baseColor = particleColor * texColor;
    // fade factor applied
    FragColor = vec4(baseColor.rgb * fadeFactor, baseColor.a * fadeFactor);
}