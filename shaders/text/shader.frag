#version 460 core
layout (location = 0) in vec2 texCoords;

layout (location = 0) out vec4 outColor;

uniform sampler2D text;
uniform vec3 textColor;

void main()
{
    const float a = clamp(texture(text, texCoords).r, 0.0, 1.0);
    outColor = vec4(textColor.rgb, a);
}
