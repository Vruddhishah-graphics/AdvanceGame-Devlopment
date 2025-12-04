#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D uSceneTexture;
uniform sampler2D uDepthTexture;
uniform vec2 uLightScreenPos;
uniform float uExposure;
uniform float uDecay;
uniform float uDensity;
uniform float uWeight;

const int NUM_SAMPLES = 100;

void main() {
    vec2 deltaTexCoord = (TexCoord - uLightScreenPos);
    deltaTexCoord *= 1.0 / float(NUM_SAMPLES) * uDensity;
    
    vec2 texCoord = TexCoord;
    float illuminationDecay = 1.0;
    vec3 color = vec3(0.0);
    
    for(int i = 0; i < NUM_SAMPLES; i++) {
        texCoord -= deltaTexCoord;
        vec3 sample = texture(uSceneTexture, texCoord).rgb;
        
        sample *= illuminationDecay * uWeight;
        color += sample;
        illuminationDecay *= uDecay;
    }
    
    color *= uExposure;
    
    vec3 sceneColor = texture(uSceneTexture, TexCoord).rgb;
    FragColor = vec4(sceneColor + color, 1.0);
}