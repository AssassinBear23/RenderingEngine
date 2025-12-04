// lighting.glsl - Reusable lighting functions

layout(std140, binding = 0) uniform LightBlock
{
    vec4 positions[4];
    vec4 directions[4];
    vec4 colors[4];
    ivec4 lightTypes[4];
    int numLights;
};

vec3 calculateLighting(vec3 fragPos, vec3 normal)
{
    vec3 ambient = vec3(0.3);
    vec3 result = ambient;
    vec3 norm = normalize(normal);

    for (int i = 0; i < numLights && i < 4; ++i)
    {
        vec3 lightPos = positions[i].xyz;
        vec3 lightDir = normalize(lightPos - fragPos);
        float diff = 0.0;
        vec3 diffuse = vec3(0.0);
        float attenuation = 1.0;
    
        int lightType = lightTypes[i].x; // Access first component
        
        if (lightType == 0) // Point light
        {
            diff = max(dot(norm, lightDir), 0.0);
            diffuse = diff * colors[i].rgb;
            
            float distance = length(lightPos - fragPos);
            attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
            
            result += diffuse * attenuation;
        }
        else if (lightType == 1) // Directional light
        {
            lightDir = normalize(-directions[i].xyz);
            diff = max(dot(norm, lightDir), 0.0);
            diffuse = diff * colors[i].rgb;
            result += diffuse;
        }
        else if (lightType == 2) // Spot light
        {
            vec3 spotDir = normalize(-directions[i].xyz);
            float theta = dot(lightDir, spotDir);
            float cutoff = 0.9;
            
            if (theta > cutoff)
            {
                diff = max(dot(norm, lightDir), 0.0);
                diffuse = diff * colors[i].rgb;
                float distance = length(lightPos - fragPos);
                attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
                result += diffuse * attenuation;
            }
        }
    }
    return result;
}