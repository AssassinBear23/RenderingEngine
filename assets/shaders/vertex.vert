#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNor;
layout (location = 2) in vec2 aUv;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

uniform mat4 mvpMatrix;
uniform mat4 modelMatrix;

out vec3 fPos;
out vec3 fNor;
out vec2 uv;
out mat3 TBN;

void main()
{
   // Calculate world position
   vec4 worldPos = modelMatrix * vec4(aPos, 1.0);
   fPos = worldPos.xyz;

   // Transform normal to world space
   mat3 normalMatrix = mat3(transpose(inverse(modelMatrix)));
   vec3 T = normalize(normalMatrix * aTangent);
   vec3 B = normalize(normalMatrix * aBitangent);
   vec3 N = normalize(normalMatrix * aNor);

   // Constuct TBN matrix for transforming tangent space normals to world space
   TBN = mat3(T, B, N);
   fNor = N;

   uv = aUv;
   gl_Position = mvpMatrix * vec4(aPos, 1.0);
}