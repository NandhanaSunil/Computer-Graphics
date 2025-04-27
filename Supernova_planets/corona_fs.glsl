#version 330 core
	in vec2 TexCoords; 
	out vec4 FragColor;
	
	uniform sampler2D coronaTexture;
	uniform float brightness;
	
	void main()
	{
		// distance from the center of the texture coordinate space 
		float dist = length(TexCoords - vec2(0.5)); // distance from center
	
		float radius = 0.5;
		float edgeSoftness = 0.05; 
	
		float circularMask = smoothstep(radius, radius - edgeSoftness, dist);
								
		
		vec4 texColor = texture(coronaTexture, TexCoords);
	
		FragColor = vec4(texColor.rgb * brightness * circularMask, texColor.a * circularMask);
	
	}