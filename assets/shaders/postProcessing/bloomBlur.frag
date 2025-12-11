#version 430 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D inputTexture;
uniform bool horizontal; // true = horizontal pass, false = vertical pass
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
    FragColor = vec4(result, 1.0);
}