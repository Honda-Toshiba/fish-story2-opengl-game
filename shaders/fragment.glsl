#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec3 WorldPos;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform vec3 skyColor;
uniform float time;
uniform bool isFloor;
uniform bool isWater;
uniform bool isSkybox;
uniform sampler2D texture_diffuse1;
uniform bool hasTexture;

// Caustics effect for underwater lighting
float caustics(vec2 uv, float time) {
    vec2 p = mod(uv * 3.14159, 3.14159) - 250.0;
    vec2 i = vec2(p);
    float c = 1.0;
    float inten = 0.005;
    
    for (int n = 0; n < 5; n++) {
        float t = time * (1.0 - (3.5 / float(n+1)));
        i = p + vec2(cos(t - i.x) + sin(t + i.y), sin(t - i.y) + cos(t + i.x));
        c += 1.0/length(vec2(p.x / (sin(i.x+t)/inten), p.y / (cos(i.y+t)/inten)));
    }
    c /= float(5);
    c = 1.17 - pow(c, 1.4);
    c = pow(abs(c), 8.0);
    return c;
}

// God rays effect
float godRays(vec3 pos, vec3 viewDir) {
    float ray = max(0.0, dot(normalize(lightPos - pos), viewDir));
    return pow(ray, 8.0) * 0.3;
}

void main()
{
    // Skybox rendering
    if (isSkybox) {
        vec3 underwaterColor = mix(
            vec3(0.05, 0.15, 0.3),  // Deep blue
            vec3(0.1, 0.4, 0.6),    // Lighter blue
            smoothstep(-50.0, 50.0, WorldPos.y)
        );
        FragColor = vec4(underwaterColor, 1.0);
        return;
    }
    
    // Normalize vectors
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Ambient lighting (underwater ambient light)
    vec3 ambient = 0.4 * lightColor;
    
    // Diffuse lighting
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Specular lighting
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = 0.5 * spec * lightColor;
    
    // Add caustics effect to floor
    vec3 result = objectColor;
    if (isFloor) {
        float causticPattern = caustics(FragPos.xz * 0.1, time);
        vec3 causticsColor = vec3(0.2, 0.5, 0.7) * causticPattern * 0.8;
        result = objectColor * (ambient + diffuse) + causticsColor + specular;
    }
    else if (isWater) {
        // Water surface with transparency and shimmer
        result = objectColor * (ambient + diffuse * 0.5);
        float shimmer = sin(FragPos.x * 2.0 + time * 3.0) * cos(FragPos.z * 2.0 + time * 2.0);
        result += vec3(0.2, 0.4, 0.6) * shimmer * 0.1;
        FragColor = vec4(result, 0.3);
        return;
    }
    else {
        // Regular object rendering (fish)
        vec3 baseColor = objectColor;
        if (hasTexture) {
            baseColor = texture(texture_diffuse1, TexCoords).rgb;
        }
        result = baseColor * (ambient + diffuse) + specular;
    }
    
    // Add god rays effect
    float rays = godRays(FragPos, viewDir);
    result += vec3(0.6, 0.8, 1.0) * rays;
    
    // Underwater fog effect (depth-based)
    float depth = length(viewPos - FragPos);
    float fogFactor = exp(-depth * 0.02);
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    
    vec3 fogColor = vec3(0.1, 0.3, 0.5); // Underwater fog color
    result = mix(fogColor, result, fogFactor);
    
    // Add slight blue tint for underwater feel
    result *= vec3(0.9, 0.95, 1.1);
    
    FragColor = vec4(result, 1.0);
}
