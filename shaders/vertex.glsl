#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out vec3 WorldPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;
uniform bool isWater;

void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);
    
    // Add wave animation to water surface
    if (isWater) {
        worldPos.y += sin(worldPos.x * 0.5 + time * 2.0) * 0.3 + 
                      cos(worldPos.z * 0.5 + time * 1.5) * 0.3;
    }
    
    FragPos = vec3(worldPos);
    WorldPos = vec3(worldPos);
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoords = aTexCoords;
    
    gl_Position = projection * view * worldPos;
}
