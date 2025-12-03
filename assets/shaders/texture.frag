#version 400 core
out vec4 FragColor;
in vec3 fNor;
in vec2 uv;
uniform sampler2D text;

void main()
{
    vec4 diffuse = texture(text, uv);
    FragColor = vec4(diffuse.r, diffuse.g, diffuse.b, 1);
}