#version 460 core

#define N_PLAYABLE_CHARACTERS 7
//#define INVALID_POSITION_INDEX 0xffffffffu

layout (location = 0) in vec3 fragColor;
layout (location = 1) in flat int vertexIndex;  // flat: Same for all fragments

layout (location = 0) out vec4 outColor;

uniform float currentTime;
uniform uint playerPositions[N_PLAYABLE_CHARACTERS];

// Constants for colors of different playable characters followed by "boeg"
// Note: Same colors as found in include/fangpp/graphics.hpp
const vec3 playerColors[N_PLAYABLE_CHARACTERS] =
{
    {173.f/255.f, 6.f/255.f, 6.f/255.f},    // red
    {27.f/255.f, 137.f/255.f, 25.f/255.f},  // green
    {25.f/255.f, 26.f/255.f, 177.f/255.f},  // blue
    {223.f/255.f, 224.f/255.f, 38.f/255.f}, // yellow
    {184.f/255.f, 95.f/255.f, 10.f/255.f},  // orange
    {97.f/255.f, 24.f/255.f, 184.f/255.f},  // purple
    {0.9f, 0.9f, 0.9f}                      // white
};

void main()
{
    uint playerIndicesAtPosition[N_PLAYABLE_CHARACTERS];
    
    uint nPlayersAtPosition = 0;
    for (int i = 0; i < N_PLAYABLE_CHARACTERS; ++i)
    {
        const uint pos = playerPositions[i];
        // TODO: Replace this magic number
        if (pos >= 133) { continue; }  // invalid player position
        
        if (pos == vertexIndex)
        {
            playerIndicesAtPosition[nPlayersAtPosition++] = i;
        }
    }
    
    if (nPlayersAtPosition == 0)
    {
        outColor = vec4(fragColor.rgb, 1.0);  // static color of vertex
    }
    else if (nPlayersAtPosition == 1)
    {
        const uint playerIndex = playerIndicesAtPosition[0];
        outColor = vec4(playerColors[playerIndex], 1.0);
    }
    else
    {
        // Animate colors of different players at same position (vertex)
        const uint varyingIndex = uint(mod(currentTime, nPlayersAtPosition));
        const uint playerIndex = playerIndicesAtPosition[varyingIndex];
        outColor = vec4(playerColors[playerIndex], 1.0);
    }
}
