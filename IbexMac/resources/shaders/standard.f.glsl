#version 330 core

#define highp

// Interpolated values from the vertex shaders
in vec2 UV;
in vec3 Position_worldspace;
in vec3 Normal_cameraspace;
in vec3 EyeDirection_cameraspace;
in vec3 LightDirection_cameraspace;

in vec4  ShadowCoord;

// Ouput data
layout (location = 0) out vec3 color;

// Values that stay constant for the whole mesh.
uniform sampler2D textureIn;
uniform mat4 MV;
//uniform vec3 LightPosition_worldspace;
uniform sampler2DShadow shadowTexture;
//uniform sampler2D shadowTexture;

void main()
{
    vec3 LightPosition_worldspace = vec3(0,5,5);
    
    // Light emission properties
	// You probably want to put them as uniforms
	vec3 LightColor = vec3(1,1,1);
	float LightPower = 100.0f;
	
	// Material properties
	vec3 MaterialDiffuseColor = texture(textureIn, UV).rgb;
	vec3 MaterialAmbientColor = vec3(0.75,0.75,0.75) * MaterialDiffuseColor;
	vec3 MaterialSpecularColor = vec3(0.3,0.3,0.3);
    
	// Distance to the light
	float distance = length( LightPosition_worldspace - Position_worldspace );
    
	// Normal of the computed fragment, in camera space
	vec3 n = normalize( Normal_cameraspace );
	// Direction of the light (from the fragment to the light)
	vec3 l = normalize( LightDirection_cameraspace );
	// Cosine of the angle between the normal and the light direction,
	// clamped above 0
	//  - light is at the vertical of the triangle -> 1
	//  - light is perpendicular to the triangle -> 0
	//  - light is behind the triangle -> 0
	float cosTheta = clamp( dot( n,l ), 0,1 );
	
	// Eye vector (towards the camera)
	vec3 E = normalize(EyeDirection_cameraspace);
	// Direction in which the triangle reflects the light
	vec3 R = reflect(-l,n);
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( E,R ), 0,1 );
	
//    float bias = 0.005;
//    float visibility = 1.0;
//    if (texture(shadowTexture, ShadowCoord.xy).x  <  ShadowCoord.z-bias) {
//        visibility = 0.5;
//    }
    float visibility = texture(shadowTexture, vec3(ShadowCoord.xy, (ShadowCoord.z/ShadowCoord.w)));
    
//    if(visibility < 0.1) color = vec3(1, 0, 0);
//    else if(visibility < 0.5) color = vec3(0, 1, 0);
//    else color = vec3(0,0,1);
    
	color = //vec3(visibility,visibility,visibility);
    // Ambient : simulates indirect lighting
    visibility * MaterialAmbientColor +
    // Diffuse : "color" of the object
    visibility * MaterialDiffuseColor * LightColor * LightPower * cosTheta / (distance*distance) +
    // Specular : reflective highlight, like a mirror
    visibility * MaterialSpecularColor * LightColor * LightPower * pow(cosAlpha,5) / (distance*distance);
}
