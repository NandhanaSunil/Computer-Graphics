#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform sampler2D texture1; 
uniform int isMoon;
uniform int isSun;  

void main()
{
    vec4 texColor = texture(texture1, TexCoord);
    
    if (isSun == 1) {
        // make the Sun brighter and self-illuminating
        vec3 sunColor = texColor.rgb * 1.0; 
        
       
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 norm = normalize(Normal);
        float rimEffect = 1.0 - max(dot(viewDir, norm), 0.0);
        vec3 glowColor = lightColor * rimEffect * 0.2;
        
        FragColor = vec4(sunColor + glowColor, texColor.a);
        //FragColor = vec4(sunColor, texColor.a);
        return; 
    }
    
    // For Earth and Moon
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);

    // Ambient Lighting
    float ambientStrength = 0.8; //0.3
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse Lighting
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular Lighting
    float specularStrength = 1.5;  
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
    vec3 specular = specularStrength * spec * lightColor;

    // Emissive Glow Effect
    float glowIntensity = smoothstep(0.3, 1.0, dot(viewDir, -lightDir));  
    vec3 emissive = glowIntensity * lightColor * 2.5; 

    // Final color
    vec3 result = (ambient + diffuse + specular + emissive) * texColor.rgb;
    FragColor = vec4(result, texColor.a);
}