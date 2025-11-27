// lighting.glsl - Reusable lighting functions

// Uniform Buffer Object for lights (matches LightData struct)
layout(std140) uniform LightBlock
{
    vec4 positions[4];
    vec4 colors[4];
    int numLights;
};

vec3 calculateLighting(vec3 fragPos, vec3 normal)
{
    vec3 ambient = vec3(0.1); // Ambient light
    vec3 result = ambient;
    vec3 norm = normalize(normal);

    for (int i = 0; i < numLights && i < 4; ++i)
    {
        vec3 lightPos = positions[i].xyz;
        vec3 lightDir = normalize(lightPos - fragPos);
        
        // Diffuse lighting
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * colors[i].rgb;
        
        // Attenuation
        float distance = length(lightPos - fragPos);
        float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
        
        result += diffuse * attenuation;
    }
    
    return result;
}