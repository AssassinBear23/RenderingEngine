#version 430 core
out vec4 FragColor;
in vec3 fPos;
in vec3 fNor;
in vec2 uv;

uniform vec4 lightColor;

void main()
{
   FragColor = lightColor;
}