#version 400 core
out vec4 FragColor;
in vec3 fPos;
in vec3 fNor;
in vec2 uv;

// Uniform Buffer Object for lights (matches LightData struct)
layout(std140) uniform LightBlock
{
    vec4 positions[4];  // CHANGED FROM vec3 TO vec4 for proper alignment
    vec4 colors[4];     // Light colors (RGB + intensity in alpha)
    int numLights;      // Number of active lights
};


vec3 calculateLighting(vec3 fragPos, vec3 normal)
{
    vec3 ambient = vec3(0.1); // Ambient light
    vec3 result = ambient;

    // Normalize the normal
    vec3 norm = normalize(normal);

    // going through the lights (max 4)
    for (int i = 0; i < numLights && i < 4; ++i)
    {
        // Light direction
        vec3 lightPos = positions[i].xyz;
        vec3 lightDir = normalize(lightPos - fragPos);

        // Diffuse lighting
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * colors[i].rgb;

        // Attenuation (distance-based falloff)
        float distance = length(lightPos - fragPos);
        float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);

        result += diffuse * attenuation;
    }

    return result;
}

void main()
{
    // Calculate lighting
    vec3 lighting = calculateLighting(fPos, fNor);

    // Base color (you can use normals as color for visualization)
    vec3 objectColor = vec3(0.8, 0.8, 0.8); // Gray object color
    // Or use normals as color for debugging:
    // vec3 objectColor = fNor * 0.5 + 0.5;

    // Final color = object color * lighting
    FragColor = vec4(objectColor * lighting, 1.0);
}