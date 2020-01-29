#version 330

in vec2 passTexCoord;
out vec4 outColour;
uniform sampler2D text;

void main() {
    outColour = texture(text, passTexCoord);
}