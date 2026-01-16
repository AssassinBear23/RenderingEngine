#version 430 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D inputTexture;
uniform sampler2D depthTexture;

uniform vec3 fogColor = vec3(0.5, 0.6, 0.7);
uniform float fogDensity = 0.05;
uniform float fogStart = 10.0;
uniform float fogEnd = 100.0;
uniform int fogMode = 0; // 0 = Linear, 1 = Exponential, 2 = Exponential Squared
uniform int debugMode = 0; // 0 = Normal, 1 = Depth, 2 = Linear Depth, 3 = Fog Factor

uniform float nearPlane = 0.1;
uniform float farPlane = 1000.0;

// Linearize depth from [0,1] to actual view-space depth
float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
}

float CalculateFogFactor(float depth)
{
    float fogFactor = 0.0;
    
    if (fogMode == 0) // Linear
    {
        fogFactor = (fogEnd - depth) / (fogEnd - fogStart);
    }
    else if (fogMode == 1) // Exponential
    {
        fogFactor = exp(-fogDensity * depth);
    }
    else if (fogMode == 2) // Exponential Squared
    {
        fogFactor = exp(-pow(fogDensity * depth, 2.0));
    }
    
    return clamp(fogFactor, 0.0, 1.0);
}

void main()
{
    vec4 sceneColor = texture(inputTexture, TexCoords);
    float depth = texture(depthTexture, TexCoords).r;
    
    // Debug modes
    if (debugMode == 1) // Raw depth
    {
        FragColor = vec4(vec3(depth), 1.0);
        return;
    }
    
    // Linearize depth
    float linearDepth = LinearizeDepth(depth);
    
    if (debugMode == 2) // Linearized depth (normalized for visualization)
    {
        float normalizedDepth = linearDepth / farPlane;
        FragColor = vec4(vec3(normalizedDepth), 1.0);
        return;
    }
    
    // Calculate fog factor
    float fogFactor = CalculateFogFactor(linearDepth);
    
    if (debugMode == 3) // Fog factor
    {
        FragColor = vec4(vec3(fogFactor), 1.0);
        return;
    }
    
    // Mix scene color with fog color
    vec3 finalColor = mix(fogColor, sceneColor.rgb, fogFactor);
    
    FragColor = vec4(finalColor, sceneColor.a);
}