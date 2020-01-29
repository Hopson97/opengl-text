#version 330

layout (location = 0) in vec2 inVertexCoord;
layout (location = 1) in vec2 inTextureCoord;

uniform mat4 modelMatrix;
uniform mat4 projectionViewMatrix;

out vec2 passTexCoord;
out vec3 passFragPosition;

void main() {
	
    gl_Position = projectionViewMatrix * modelMatrix * vec4(inVertexCoord, 0.0f, 1.0);
    
    passTexCoord = inTextureCoord;
}