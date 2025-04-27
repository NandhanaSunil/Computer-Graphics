#version 330 core
in vec4 particleColor;
in float particleLife;
out vec4 FragColor;
uniform sampler2D particleTexture;
uniform float uFadeExponent;
void main()
{
    vec4 texColor = texture(particleTexture, gl_PointCoord);
    if (texColor.a < 0.02) discard;
    float fadeFactor = pow(particleLife, uFadeExponent);
    vec4 baseColor = particleColor * texColor;
    FragColor = vec4(baseColor.rgb * fadeFactor, baseColor.a * fadeFactor);
}