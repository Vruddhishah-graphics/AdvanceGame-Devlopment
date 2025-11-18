#version 330 core
in vec3 vPos;
in vec3 vNormal;
in vec2 vTexCoord;

out vec4 FragColor;

uniform sampler2D uTexture;
uniform bool uUseTexture;
uniform vec3 uBaseColor;
uniform float uEmissive;
uniform vec3 uViewPos;

// Lights
struct Light {
    vec3 position;
    vec3 color;
};

uniform Light uL1;
uniform Light uL2;

// Material properties
uniform float uKa;
uniform float uKd;
uniform float uKs;
uniform float uShine;

vec3 CalculateLight(vec3 lightPos, vec3 lightColor, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 baseColor) {
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), uShine);
    vec3 specular = spec * lightColor * 0.5;

    return (diffuse * baseColor) + specular;
}

void main() {
    vec3 norm = normalize(vNormal);
    vec3 viewDir = normalize(uViewPos - vPos);

    vec3 baseColor;
    if (uUseTexture) {
        baseColor = texture(uTexture, vTexCoord).rgb;
    } else {
        baseColor = uBaseColor;
    }
    
    vec3 ambient = uKa * baseColor;
    vec3 result = ambient;

    result += CalculateLight(uL1.position, uL1.color, norm, vPos, viewDir, baseColor);
    result += CalculateLight(uL2.position, uL2.color, norm, vPos, viewDir, baseColor);
    
    vec3 emissive = uEmissive * baseColor;
    result += emissive;

    FragColor = vec4(result, 1.0);
}