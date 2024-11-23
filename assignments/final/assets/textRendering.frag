#version 330

in vec2 TextureCooridnates;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;

void main()
{
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TextureCooridnates));
    color = vec4(textColor, 1.0) * sampled;
}