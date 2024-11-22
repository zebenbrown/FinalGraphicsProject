#version 330

layout (location = 0) in vec3 aPosition;


uniform mat4 modelMatrix;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 offset;

void main()
{
    gl_Position = projection * view * modelMatrix * vec4(aPosition.x + offset.x, aPosition.y, aPosition.z, 1.0f);

}