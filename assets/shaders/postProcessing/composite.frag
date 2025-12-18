#version 430 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D sceneTexture;
uniform sampler2D bloomTexture;

void main()
{
    vec4 sceneResult = texture(sceneTexture, TexCoords);
    vec4 bloomResult = texture(bloomTexture, TexCoords);

    FragColor = sceneResult + bloomResult;
}