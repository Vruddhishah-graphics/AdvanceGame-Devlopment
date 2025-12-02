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

// Light structure
struct Light {
    vec3 position;
    vec3 color;
};

uniform Light uL1;
uniform Light uL2;

// Blinn-Phong material properties
uniform float uKa;  // Ambient coefficient
uniform float uKd;  // Diffuse coefficient
uniform float uKs;  // Specular coefficient
uniform float uShine;  // Shininess/specular exponent

// Blinn-Phong lighting calculation
vec3 CalculateBlinnPhong(vec3 lightPos, vec3 lightColor, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 baseColor) {
    // Ambient component (constant)
    vec3 ambient = uKa * lightColor * baseColor;
    
    // Direction from fragment to light
    vec3 lightDir = normalize(lightPos - fragPos);
    
    // Diffuse component (Lambertian reflectance)
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = uKd * diff * lightColor * baseColor;
    
    // Specular component (Blinn-Phong)
    // Use halfway vector instead of reflection vector (more efficient)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), uShine);
    vec3 specular = uKs * spec * lightColor;
    
    // Attenuation (distance falloff)
    float distance = length(lightPos - fragPos);
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
    
    return (ambient + diffuse + specular) * attenuation;
}

void main() {
    // Normalize the interpolated normal
    vec3 norm = normalize(vNormal);
    
    // View direction (from fragment to camera)
    vec3 viewDir = normalize(uViewPos - vPos);
    
    // Get base color (from texture or uniform)
    vec3 baseColor;
    if (uUseTexture) {
        vec3 texColor = texture(uTexture, vTexCoord).rgb;
        baseColor = texColor;
    } else {
        baseColor = uBaseColor;
    }
    
    // Calculate lighting from both lights using Blinn-Phong model
    vec3 result = vec3(0.0);
    result += CalculateBlinnPhong(uL1.position, uL1.color, norm, vPos, viewDir, baseColor);
    result += CalculateBlinnPhong(uL2.position, uL2.color, norm, vPos, viewDir, baseColor);
    
    // Add emissive component (for glowing objects like power-ups)
    vec3 emissive = uEmissive * baseColor;
    result += emissive;
    
    // Gamma correction for better visual quality
    result = pow(result, vec3(1.0/2.2));
    
    FragColor = vec4(result, 1.0);
}