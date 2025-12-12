#version 430 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D inputTexture;
uniform float threshold; // Brightness threshold (e.g., 1.0)

void main()
{
    vec3 color = texture(inputTexture, TexCoords).rgb;
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722)); // Luminance
    
    if(brightness > threshold)
        FragColor = vec4(color, 1.0);
    else
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}