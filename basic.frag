#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;

out vec4 fColor;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;
//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;
// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

//components
vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;

uniform float fogDensity;

float constant = 1.0f;
float linear = 0.0045f;
float quadratic = 0.0075f;
float shininess = 32.0f;

uniform vec3 lightPos;
uniform vec3 lightPos1;
uniform float luminaDir;

in vec4 fPosEye;
in vec4 lightPosEye;
in vec4 lightPosEye1;

void computeDirLight()
{
    //compute eye space coordinates
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);

    //normalize light direction
    vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

    //compute view direction (in eye coordinates, the viewer is situated at the origin
    vec3 viewDir = normalize(- fPosEye.xyz);

    //compute ambient light
    ambient = ambientStrength * lightColor;

    //compute diffuse light
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

    //compute specular light
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    specular = specularStrength * specCoeff * lightColor;
}

float computeFog()
{
 //float fogDensity = 0.05f;
 float fragmentDistance = length(fPosition);
 float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

 return clamp(fogFactor, 0.0f, 1.0f);
}

void computePointLight()
{	
	float dist = length(lightPosEye.xyz - vec3(fPosEye));
    float att = 1.0f / (constant + linear * dist + quadratic * (dist * dist));
	
	vec3 cameraPosEye = vec3(0.0f);
	
	vec3 normalEye = normalize(fNormal);
	
	vec3 lightDirN = normalize(lightPosEye.xyz - vec3(fPosEye.x,fPosEye.y,fPosEye.z));
	
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
	
	vec3 halfVector = normalize(lightDirN + viewDirN);
	
	ambient += att * ambientStrength * lightColor;
	
	diffuse += att * max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), shininess);
	specular += att * specularStrength * specCoeff * lightColor;	
}
void computePointLight1()
{	
	float dist = length(lightPosEye1.xyz - vec3(fPosEye));
    float att = 1.0f / (constant + linear * dist + quadratic * (dist * dist));
	
	vec3 cameraPosEye = vec3(0.0f);
	
	vec3 normalEye = normalize(fNormal);
	
	vec3 lightDirN = normalize(lightPosEye1.xyz - vec3(fPosEye.x,fPosEye.y,fPosEye.z));
	
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
	
	vec3 halfVector = normalize(lightDirN + viewDirN);
	
	ambient += att * ambientStrength * lightColor;
	
	diffuse += att * max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), shininess);
	specular += att * specularStrength * specCoeff * lightColor;	
}

void main() 
{
    if (luminaDir == 0){computeDirLight();}
     computePointLight();
     computePointLight1();

    //compute final vertex color
    vec3 color = min((ambient + diffuse) * texture(diffuseTexture, fTexCoords).rgb + specular * texture(specularTexture, fTexCoords).rgb, 1.0f);

    float fogFactor = computeFog();
    vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
    fColor = mix(fogColor, vec4(color, 0.1f), fogFactor);

    //fColor = vec4(color, 1.0f);
}
