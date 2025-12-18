// lighting.glsl - Reusable lighting functions

uniform sampler2D shadowMap;

layout(std140, binding = 0) uniform LightBlock
{
    vec4 positions[4];
    vec4 directions[4];
    vec4 colors[4];     // .rgb = color, .w = intensity
    ivec4 lightTypes[4];
    int numLights;
};

// From openGL tutorial
float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    // Perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    
    // Get closest depth value from light's perspective
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    
    // Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    
    // Calculate bias (from tutorial "Shadow acne")
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    
    // PCF (Percentage Closer Filtering) for soft shadows
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    
    // Keep fragments outside light frustum in light (from tutorial "Over sampling")
    if(projCoords.z > 1.0)
        shadow = 0.0;
    
    return shadow;
}

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
        float intensity = colors[i].w;  // Get intensity from color alpha channel
    
        int lightType = lightTypes[i].x; // Access first component
        
        if (lightType == 0) // Point light
        {
            diff = max(dot(norm, lightDir), 0.0);
            diffuse = diff * colors[i].rgb * intensity;
            
            float distance = length(lightPos - fragPos);
            attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
            
            result += diffuse * attenuation;
        }
        else if (lightType == 1) // Directional light
        {
            lightDir = normalize(-directions[i].xyz);
            diff = max(dot(norm, lightDir), 0.0);
            diffuse = diff * colors[i].rgb * intensity;
            
            // Calculate shadow for directional light (first light only for now)
            float shadow = 0.0;
            if (i == 0)
            {
                shadow = ShadowCalculation(FragPosLightSpace, norm, lightDir);
            }
            
            result += diffuse * (1.0 - shadow);
        }
        else if (lightType == 2) // Spot light
        {
            vec3 spotDir = normalize(-directions[i].xyz);
            float theta = dot(lightDir, spotDir);
            float cutoff = 0.9;
            
            if (theta > cutoff)
            {
                diff = max(dot(norm, lightDir), 0.0);
                diffuse = diff * colors[i].rgb * intensity;
                float distance = length(lightPos - fragPos);
                attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
                result += diffuse * attenuation;
            }
        }
    }
    
    return result;
}