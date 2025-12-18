#version 430 core
out vec4 FragColor;
in vec3 fPos;
in vec3 fNor;
in vec2 uv;
in mat3 TBN;
in vec4 FragPosLightSpace;

uniform sampler2D albedoMap;
uniform sampler2D aoMap;
uniform sampler2D normalMap;
uniform bool useNormalMap;

#include "lighting.glsl"

void main()
{
    vec3 albedo = texture(albedoMap, uv).rgb;
    float ao = texture(aoMap, uv).r;


    vec3 normal = normalize(fNor);
    // Get normal from map
    if (useNormalMap) {
        vec3 tangentNormal = texture(normalMap, uv).rgb;
        tangentNormal = tangentNormal * 2.0 - 1.0; // Transform from [0,1] to [-1,1]
        normal = normalize(TBN * tangentNormal);
    }

    // Calculate lighting with calculated normal
    vec3 lighting = calculateLighting(fPos, normal);

    FragColor = vec4(albedo * lighting * ao, 1.0);
}