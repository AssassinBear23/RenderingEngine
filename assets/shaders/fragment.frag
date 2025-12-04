#version 430 core
out vec4 FragColor;
in vec3 fPos;
in vec3 fNor;
in vec2 uv;

void main()
{
   FragColor = vec4(fNor.x, fNor.y, fNor.z, 1);
}