#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;

void main()
{    
    vec4 sampled = texture(text, TexCoords);
    
    // FIX: Use the Red channel brightness as the Alpha value.
    // Since the font is White (1,1,1) on Black (0,0,0):
    // - Black pixels have R=0 -> Alpha becomes 0 (Transparent)
    // - White pixels have R=1 -> Alpha becomes 1 (Visible)
    float alpha = sampled.r;
    
    if(alpha < 0.1)
        discard;
        
    color = vec4(textColor, alpha);
}