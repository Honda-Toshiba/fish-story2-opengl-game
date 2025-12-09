#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec3 WorldPos;

// Uniforms
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform vec3 skyColor;

// Flags
uniform bool hasTexture;
uniform bool isFloor;
uniform bool isWater;
uniform bool isSkybox;
uniform bool isSun;
uniform bool isGlowingFish;
uniform bool isPowerUp;

uniform sampler2D texture_diffuse1;
uniform float time;
uniform vec3 powerUpColor;

// Caustics function
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
    // 1. SUN LOGIC (The "Glow")
    // If this is the sun/moon itself, render it bright and unlit.
    if (isSun) {
        // Simple bloom effect: center is white, edges are colored
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 norm = normalize(Normal);
        float glow = pow(max(dot(norm, viewDir), 0.0), 3.0);
        vec3 finalColor = mix(objectColor, vec3(1.0), glow);
        
        FragColor = vec4(finalColor, 1.0);
        return; 
    }

    // 2. Skybox rendering
    if (isSkybox) {
        vec3 underwaterColor = mix(
            vec3(0.05, 0.15, 0.3),  // Deep blue
            vec3(0.1, 0.4, 0.6),    // Lighter blue
            smoothstep(-50.0, 50.0, WorldPos.y)
        );
        FragColor = vec4(underwaterColor, 1.0);
        return;
    }
    
    // --- REAL LIGHT SOURCE PHYSICS ---
    
    // Normalize vectors
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Calculate Distance for "Point Light" behavior
    float distance = length(lightPos - FragPos);
    
    // Attenuation (Falloff)
    // 1.0 / (constant + linear * dist + quadratic * dist^2)
    // We use very small numbers because your world scale is large (Sun is 100 units away)
    float attenuation = 1.0 / (1.0 + 0.001 * distance + 0.00002 * distance * distance);
    
    // Ambient lighting (Base brightness)
    vec3 ambient = 0.3 * lightColor;
    
    // Diffuse lighting (Directional brightness)
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Specular lighting (Shiny reflections)
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64.0); // Sharper highlight
    vec3 specular = 1.0 * spec * lightColor; 
    
    // Combine Lighting components and apply Attenuation
    // Note: Ambient is usually not attenuated in simple engines, but attenuating it
    // helps the night cycle feel darker when far from the moon.
    vec3 lighting = (ambient + diffuse + specular) * attenuation;
    
    // --- OBJECT MATERIAL LOGIC ---
    
    vec3 result;
    float alpha = 1.0;

    if (isFloor) {
        // Floor gets Caustics + Lighting
        float causticPattern = caustics(FragPos.xz * 0.1, time);
        vec3 causticsColor = vec3(0.2, 0.5, 0.7) * causticPattern * 0.8;
        
        // Multiply floor color by lighting, then add caustics on top
        result = objectColor * lighting + causticsColor;
    }
    else if (isWater) {
        // Water Surface gets Lighting + Shimmer
        float shimmer = sin(FragPos.x * 2.0 + time * 3.0) * cos(FragPos.z * 2.0 + time * 2.0);
        vec3 shimmerColor = vec3(0.2, 0.4, 0.6) * shimmer * 0.1;
        
        result = objectColor * lighting + shimmerColor;
        alpha = 0.4; // Transparency
    }
    else {
        // Regular objects (Fish, Shells, Enemies)
        vec3 baseColor = objectColor;
        if (hasTexture) {
            baseColor = texture(texture_diffuse1, TexCoords).rgb;
        }
        result = baseColor * lighting;
        
        // Add subtle glow to small fish
        if (isGlowingFish) {
            vec3 glowColor = vec3(0.3, 0.5, 0.7); // Cyan-ish glow
            float glowIntensity = 0.15; // Subtle glow
            result += glowColor * glowIntensity;
        }
        
        // Add powerup glow (green for speed, yellow for double score)
        if (isPowerUp) {
            float glowIntensity = 0.4 + 0.2 * sin(time * 3.0); // Pulsing glow
            result += powerUpColor * glowIntensity;
        }
    }
    
    // --- POST-PROCESSING EFFECTS ---

    // God rays effect
    float rays = godRays(FragPos, viewDir);
    result += vec3(0.6, 0.8, 1.0) * rays;
    
    // Underwater fog effect (Depth-based)
    float viewDist = length(viewPos - FragPos);
    float fogFactor = exp(-viewDist * 0.02);
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    
    vec3 fogColor = vec3(0.1, 0.3, 0.5); 
    result = mix(fogColor, result, fogFactor);
    
    // Final Blue Tint (Underwater Atmosphere)
    result *= vec3(0.9, 0.95, 1.1);
    
    FragColor = vec4(result, alpha);
}