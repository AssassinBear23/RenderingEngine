#version 430 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D inputTexture;
uniform sampler2D sceneTexture;  // Original scene (only used on final pass)
uniform bool horizontal;         // true = horizontal pass, false = vertical pass
uniform bool isFinalPass;        // true on the last blur pass
uniform float intensity;         // Bloom intensity for final combine
uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{
    vec2 tex_offset = 1.0 / textureSize(inputTexture, 0); // gets size of single texel
    vec3 result = texture(inputTexture, TexCoords).rgb * weight[0];
    
    if(horizontal)
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(inputTexture, TexCoords + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(inputTexture, TexCoords - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(inputTexture, TexCoords + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            result += texture(inputTexture, TexCoords - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        }
    }
    
    // If this is the final pass, combine with original scene
    if (isFinalPass)
    {
        vec3 sceneColor = texture(sceneTexture, TexCoords).rgb;
        result = sceneColor + result * intensity;
    }
    
    FragColor = vec4(result, 1.0);
}