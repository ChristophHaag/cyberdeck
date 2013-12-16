#version 330

#define highp

// Interpolated values from the vertex shaders
in vec2 UV;
in vec3 Position_worldspace;

// Ouput data
layout (location = 0) out vec3 color;

// Values that stay constant for the whole mesh.
uniform sampler2D textureIn;

void main()
{
	color = texture(textureIn, UV).rgb;
}
