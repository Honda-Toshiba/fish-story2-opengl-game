#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec3 WorldPos;

// Light structure for multiple lights
struct Light {
    vec3 position;
    vec3 color;
    float intensity;
    float radius;
};

// Uniforms
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform vec3 skyColor;
uniform vec3 ambientLight;

// Multiple lights support (for Level 2 anglerfish)
#define MAX_LIGHTS 8
uniform Light lights[MAX_LIGHTS];
uniform int numLights;

// Flags
uniform bool hasTexture;
uniform bool isFloor;
uniform bool isWater;
uniform bool isSkybox;
uniform bool isSun;
uniform bool isGlowingFish;
uniform bool isPowerUp;
uniform bool isCave;
uniform bool isGlowing;
uniform float glowIntensity;

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

// Calculate lighting from multiple point lights
vec3 calculateMultipleLights(vec3 norm, vec3 viewDir, vec3 baseColor) {
    vec3 result = vec3(0.0);
    
    // Add contribution from each light
    for (int i = 0; i < numLights && i < MAX_LIGHTS; i++) {
        vec3 lightDir = normalize(lights[i].position - FragPos);
        float distance = length(lights[i].position - FragPos);
        
        // Very generous attenuation for maximum visibility
        float attenuation = lights[i].intensity / (1.0 + 0.02 * distance + 0.001 * distance * distance);
        attenuation *= smoothstep(lights[i].radius * 2.0, 0.0, distance);
        
        // Diffuse - use softer falloff (wrap lighting)
        float diff = max(dot(norm, lightDir), 0.0);
        float wrappedDiff = (diff + 0.3) / 1.3; // Wrap lighting to soften shadows
        vec3 diffuse = wrappedDiff * lights[i].color;
        
        // Specular - significantly increased for much brighter wall reflection
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16.0); // Reduced shininess for broader highlights
        vec3 specular = 3.0 * spec * lights[i].color; // Increased from 1.2 to 3.0 for much brighter walls
        
        result += (diffuse + specular) * attenuation;
    }
    
    // Cap the total light contribution to prevent over-brightening with many lights
    // This prevents the scene from becoming washed out when you collect all anglerfish
    float maxBrightness = 2.5; // Increased from 1.2 to 2.5 for much brighter cave
    float currentBrightness = length(result);
    if (currentBrightness > maxBrightness) {
        result = normalize(result) * maxBrightness;
    }
    
    return result * baseColor;
}

void main()
{
    // 1. SUN LOGIC (The "Glow")
    if (isSun) {
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 norm = normalize(Normal);
        float glow = pow(max(dot(norm, viewDir), 0.0), 3.0);
        vec3 finalColor = mix(objectColor, vec3(1.0), glow);
        FragColor = vec4(finalColor, 1.0);
        return; 
    }
    
    // 2. Glowing objects (Anglerfish)
    if (isGlowing) {
        vec3 glowColor = objectColor * (1.0 + glowIntensity);
        FragColor = vec4(glowColor, 1.0);
        return;
    }

    // 3. Skybox rendering
    if (isSkybox) {
        vec3 underwaterColor = mix(
            vec3(0.05, 0.15, 0.3),
            vec3(0.1, 0.4, 0.6),
            smoothstep(-50.0, 50.0, WorldPos.y)
        );
        FragColor = vec4(underwaterColor, 1.0);
        return;
    }
    
    // --- LIGHTING SETUP ---
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Calculate Distance for "Point Light" behavior
    float distance = length(lightPos - FragPos);
    float attenuation = 1.0 / (1.0 + 0.001 * distance + 0.00002 * distance * distance);
    
    // Base lighting components
    vec3 ambient = vec3(0.0);
    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);
    
    // Use multiple lights if in cave mode (Level 2)
    if (numLights > 0) {
        // Dark cave ambient
        ambient = ambientLight;
        
        // Add multiple point lights
        vec3 multiLightResult = calculateMultipleLights(norm, viewDir, vec3(1.0));
        diffuse = multiLightResult;
    } else {
        // Level 1 lighting (sun/moon)
        ambient = 0.3 * lightColor;
        
        float diff = max(dot(norm, lightDir), 0.0);
        diffuse = diff * lightColor;
        
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64.0);
        specular = 1.0 * spec * lightColor;
        
        diffuse *= attenuation;
        specular *= attenuation;
    }
    
    vec3 lighting = ambient + diffuse + specular;
    
    // --- OBJECT MATERIAL LOGIC ---
    vec3 result;
    float alpha = 1.0;

    if (isCave) {
        // Cave walls/floor - simple rocky appearance
        result = objectColor * lighting;
    }
    else if (isFloor) {
        // Ocean floor gets Caustics + Lighting
        float causticPattern = caustics(FragPos.xz * 0.1, time);
        vec3 causticsColor = vec3(0.2, 0.5, 0.7) * causticPattern * 0.8;
        result = objectColor * lighting + causticsColor;
    }
    else if (isWater) {
        // Water Surface gets Lighting + Shimmer
        float shimmer = sin(FragPos.x * 2.0 + time * 3.0) * cos(FragPos.z * 2.0 + time * 2.0);
        vec3 shimmerColor = vec3(0.2, 0.4, 0.6) * shimmer * 0.1;
        result = objectColor * lighting + shimmerColor;
        alpha = 0.4;
    }
    else {
        // Regular objects (Fish, Shells, Enemies)
        vec3 baseColor = objectColor;
        if (hasTexture) {
            baseColor = texture(texture_diffuse1, TexCoords).rgb;
        }
        
        // Apply lighting
        result = baseColor * lighting;
        
        // Add subtle glow to small fish
        if (isGlowingFish) {
            vec3 glowColor = vec3(0.3, 0.5, 0.7); // Cyan-ish glow
            float fishGlowIntensity = 0.15; // Subtle glow
            result += glowColor * fishGlowIntensity;
        }
        
        // Add powerup glow (green for speed, yellow for double score)
        if (isPowerUp) {
            float powerUpGlowIntensity = 0.4 + 0.2 * sin(time * 3.0); // Pulsing glow
            result += powerUpColor * powerUpGlowIntensity;
        }
        
        // Add glow effect for bioluminescent objects (Anglerfish, coins)
        if (isGlowing) {
            // Add very subtle emissive glow that doesn't wash out the model
            // Use a soft cyan-blue glow color
            vec3 glowColor = vec3(0.3, 0.7, 0.9) * glowIntensity;
            result += glowColor * 0.5; // Even more subtle addition
        }
    }
    
    // --- POST-PROCESSING EFFECTS ---
    
    // God rays effect (only for Level 1)
    if (numLights == 0) {
        float rays = godRays(FragPos, viewDir);
        result += vec3(0.6, 0.8, 1.0) * rays;
    }
    
    // Fog effect
    float viewDist = length(viewPos - FragPos);
    float fogFactor;
    vec3 fogColor;
    
    if (isCave || numLights > 0) {
        // Very light cave fog for playability
        fogFactor = exp(-viewDist * 0.01);  // Very light fog
        fogColor = vec3(0.15, 0.15, 0.2);   // Much lighter fog color
    } else {
        // Underwater fog (Level 1)
        fogFactor = exp(-viewDist * 0.02);
        fogColor = vec3(0.1, 0.3, 0.5);
    }
    
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    result = mix(fogColor, result, fogFactor);
    
    // Blue tint for underwater (Level 1 only)
    if (numLights == 0 && !isCave) {
        result *= vec3(0.9, 0.95, 1.1);
    }
    
    FragColor = vec4(result, alpha);
}