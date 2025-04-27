#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform sampler2D texture1; // Texture
uniform int isSun;  
uniform int isMerc;
uniform float u_time;

void main()
{
    // to get the texture color 
    vec4 texColor = texture(texture1, TexCoord);
    
    // if sun, we render it emissive
    if (isSun == 1) {
    vec4 texColor = texture(texture1, TexCoord);
    //texColor = vec4(1.0, 1.0, 1.0, 1.0);
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    float timeElapsed = max(0.0, u_time - 5.0);
    float t = 1.0 - exp(-timeElapsed * 0.1);
    float rimEffect = 1.0 - max(dot(viewDir, norm), 0.0);
    vec3 glowColor = vec3(1.0, 0.7, 0.4) * rimEffect * (0.5);

    vec3 redGiantTint = vec3(1.0, 0.3, 0.1);
    // vec3 sunColor = mix(redGiantTint, texColor.rgb * t, t);
    vec3 sunColor = mix(redGiantTint, texColor.rgb, texColor.rgb);

    vec3 finalSunColor = clamp(sunColor + glowColor, 0.0, 1.0);
    FragColor = vec4(finalSunColor, 1.0); 
    return;
}
    
    // for other planets
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);

    // ambient lighting
    float ambientStrength = 0.8;
    vec3 ambient = ambientStrength * lightColor;

    // diffuse lighting
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular Lighting
    float specularStrength = 0.70;  
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
    vec3 specular = specularStrength * spec * lightColor;

    // emissive glow effect - only for non-sun objects
    float glowIntensity = smoothstep(0.3, 1.0, dot(viewDir, -lightDir));  
    vec3 emissive = glowIntensity * lightColor * 2.5; 

    // final color
    vec3 result = (ambient + diffuse + specular) * texColor.rgb;
    FragColor = vec4(result, texColor.a);

    if (isMerc ==1){
    float timeElapsed = max(0.0, u_time - 5.0);
    float t = 1.0 - exp(-timeElapsed * 0.1);
    float burnfactor = clamp(t*2.0, 0.0, 1.0);
    vec3 burnTint = vec3(1.0, 0.2, 0.1); // burning glow
    FragColor.rgb = mix( FragColor.rgb, burnTint,t);
    }
}