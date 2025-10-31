#version 400 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNor;
layout (location = 2) in vec2 aUv;
out vec3 fPos;
out vec3 fNor;
out vec2 uv;

void main()
{
   fPos = aPos;
   fNor = aNor;
   uv = aUv;
   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
}