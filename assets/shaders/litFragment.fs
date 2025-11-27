#version 400 core
out vec4 FragColor;
in vec3 fPos;
in vec3 fNor;
in vec2 uv;

#include "lighting.glsl"

void main()
{
    // Calculate lighting
    vec3 lighting = calculateLighting(fPos, fNor);

    vec3 objectColor = vec3(0.8, 0.8, 0.8); // Gray object color

    FragColor = vec4(objectColor * lighting, 1.0);
}