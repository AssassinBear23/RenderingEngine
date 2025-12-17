#version 430 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D inputTexture;

void main()
{
    vec4 color = texture(inputTexture, TexCoords);

    FragColor = vec4(1 - color.rgb, 1);
}