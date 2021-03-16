#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;

out vec4 fColor;

//lighting
uniform	mat3 normalMatrix;
uniform	vec3 lightDir;
uniform	vec3 lightColor;

uniform int fog;
uniform int colorReflection;

//texture
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 64.0f;
float quadratic = 0.032;
float linear = 0.09f;
float constant = 1.0f;

float ambientStrengthPoint = 0.5f;
float specularStrengthPoint = 0.5f;
float shininessPoint = 32.0f;

vec3 colorFromSkybox;
uniform samplerCube skybox;

uniform mat4 view;

void computeLightComponents()
{		
	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
	
	//transform normal
	vec3 normalEye = normalize(fNormal);	
	
	//compute light direction
	vec3 lightDirN = normalize(lightDir);
	
	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
		
	//compute ambient light
	ambient = ambientStrength * lightColor;
	
	//compute diffuse light
	diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	
	//compute specular light
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
	specular = specularStrength * specCoeff * lightColor;

	//----------------------------------------skybox--------------------------------------------
	
	vec3 reflectionSkyBox = reflect(viewDirN, normalEye);
	colorFromSkybox = vec3(texture(skybox, reflectionSkyBox));

}

float computeShadow()
{	
	// perform perspective divide
    vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    if(normalizedCoords.z > 1.0f)
        return 0.0f;
    // Transform to [0,1] range
    normalizedCoords = normalizedCoords * 0.5f + 0.5f;
    // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, normalizedCoords.xy).r;    
    // Get depth of current fragment from light's perspective
    float currentDepth = normalizedCoords.z;
    // Check whether current frag pos is in shadow
    float bias = 0.005f;
    float shadow = currentDepth - bias> closestDepth  ? 1.0f : 0.0f;

    return shadow;	
}

float computeFog() 
{
	float fogDensity = 0.01f;
	float fragmentDistance = length(fPosEye); 
	float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));
	return clamp(fogFactor, 0.0f, 1.0f);
}

vec3 computePointLight(vec4 lightPosEye){
	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
	
	//transform normal
	vec3 normalEye = normalize(fNormal);	
	
	//compute light direction
	vec3 lightDirN = normalize(lightDir);
	
	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
		
	//compute ambient light
	vec3 ambientPoint = ambientStrengthPoint * lightColor;
	
	//compute diffuse light
	vec3 diffusePoint = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	
	//compute specular light
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininessPoint);
	vec3 specularPoint = specularStrengthPoint * specCoeff * lightColor;

	float distance = length(lightPosEye.xyz - fPosEye.xyz);
	float att = 1.0f / (constant + linear * distance + quadratic * distance * distance);
	return (ambientPoint + diffusePoint + specularPoint) * att;
}

void main() 
{
	computeLightComponents();
	
	vec3 baseColor = vec3(0.9f, 0.35f, 0.0f);//orange
	float shadow = computeShadow();
	
	ambient *= texture(diffuseTexture, fTexCoords).rgb;
	diffuse *= texture(diffuseTexture, fTexCoords).rgb;
	specular *= texture(specularTexture, fTexCoords).rgb;

	vec3 color = min((ambient + (1.0f - shadow)*diffuse) + (1.0f - shadow)*specular, 1.0f); 
	

	vec4 lightPosEye = view * vec4(-2, 1.5f, 0, 1.0f);
	vec3 light = ambient+diffuse+specular;
	light += computePointLight(lightPosEye);

	//fColor = vec4(color, 1.0f);
	vec4 colorWithShadow = vec4(color,1.0f);
	fColor = min(colorWithShadow * vec4(light, 1.0f), 1.0f);
	
	if (fog==1){
		float fogFactor = computeFog();
		vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
		fColor = fogColor*(1-fogFactor)+(vec4(color, 1.0f))*fogFactor;
	
		vec4 colorFromTexture = texture(diffuseTexture, fTexCoords);
		if(colorFromTexture.a < 0.1)
			discard;

		fColor = colorFromTexture;
	}

	if(colorReflection==1){
		//for reflection
		//fColor = vec4(colorFromSkybox, 1.0f);
		fColor = vec4(color, 1.0f);
	}
	
}
