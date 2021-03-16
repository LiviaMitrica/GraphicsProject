#version 410 core

in vec3 textureCoordinates;
out vec4 color;

uniform samplerCube skybox;

uniform int fog;

float computeFog()
{
	float fogDensity = 0.09f;
	float fragmentDistance = 21.0f;
	float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));
	return clamp(fogFactor, 0.0f, 1.0f);
}

void main()
{
    vec4 auxColor = texture(skybox, textureCoordinates);

	if(fog==0)
	{
		color = auxColor;
	}
	else
	{
		float fogFactor = computeFog();
		vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
		color = mix(fogColor, auxColor, fogFactor);
	}
}
