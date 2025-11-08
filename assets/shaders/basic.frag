#version 330 core
in vec3 vPos;
in vec3 vNormal;
out vec4 FragColor;

struct Light { vec3 position; vec3 color; };

uniform vec3 uViewPos;
uniform vec3 uBaseColor;
uniform float uEmissive;
uniform Light uL1;
uniform Light uL2;

uniform float uKa;
uniform float uKd;
uniform float uKs;
uniform float uShine;

void main(){
    vec3 N = normalize(vNormal);
    vec3 V = normalize(uViewPos - vPos);

    vec3 L1dir = normalize(uL1.position - vPos);
    vec3 H1    = normalize(L1dir + V);
    float diff1 = max(dot(N,L1dir), 0.0);
    float spec1 = pow(max(dot(N,H1), 0.0), uShine);

    vec3 L2dir = normalize(uL2.position - vPos);
    vec3 H2    = normalize(L2dir + V);
    float diff2 = max(dot(N,L2dir), 0.0);
    float spec2 = pow(max(dot(N,H2), 0.0), uShine);

    vec3 ambient  = uKa * uBaseColor;
    vec3 diffuse  = uKd * uBaseColor * (uL1.color * diff1 + uL2.color * diff2);
    vec3 specular = uKs * (uL1.color * spec1 + uL2.color * spec2);

    vec3 emissive = uEmissive * uBaseColor;
    FragColor = vec4(ambient + diffuse + specular + emissive, 1.0);
}
