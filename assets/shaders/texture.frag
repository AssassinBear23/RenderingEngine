#version 430 core

// MRT outputs
layout (location = 0) out vec4 FragColor;     // Scene color
layout (location = 1) out vec4 BrightColor;   // Bright pixels for bloom

in vec3 fNor;
in vec2 uv;
uniform sampler2D text;

// Bloom threshold
uniform float bloomThreshold = 1.0;

void main()
{
    vec4 diffuse = texture(text, uv);
    FragColor = vec4(diffuse.rgb, 1.0);
    
    // Calculate brightness and output to bloom buffer
    float brightness = dot(diffuse.rgb, vec3(0.2126, 0.7152, 0.0722));
    if (brightness > bloomThreshold)
        BrightColor = vec4(diffuse.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}