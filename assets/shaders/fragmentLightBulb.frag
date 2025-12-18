#version 430 core
out vec4 FragColor;
in vec3 fPos;
in vec3 fNor;
in vec2 uv;

uniform vec4 lightColor;
uniform float intensity;

void main()
{
   FragColor = lightColor * intensity;
}