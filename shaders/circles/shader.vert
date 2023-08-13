#version 460 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 instanceOffset;
layout (location = 2) in vec3 instanceColor;

layout (location = 0) out vec3 fragColor;

uniform mat4 proj;

void main()
{
    gl_Position = proj * vec4(aPos.xy + instanceOffset.xy, 0.0, 1.0);
    fragColor = instanceColor;
}
