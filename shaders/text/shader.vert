#version 460 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTex;

layout (location = 0) out vec2 texCoords;

uniform mat4 proj;

void main()
{
    gl_Position = proj * vec4(aPos.xy, 0.0, 1.0);
    texCoords = aTex;
}
