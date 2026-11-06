#version 330 core

in vec3 vPosition;
in vec3 vNormal;
in vec2 vUv;

uniform sampler2D uTexture;
uniform vec3 uColor;
uniform vec3 uEye;
uniform vec3 uLight;
uniform float uTextureAmount;
uniform float uEmission;
uniform float uAlpha;

out vec4 fragmentColor;

void main() {
    vec3 normal = normalize(vNormal);
    vec3 toLight = uLight - vPosition;
    float distanceToLight = max(length(toLight), 0.001);
    vec3 lightDirection = toLight / distanceToLight;
    vec3 viewDirection = normalize(uEye - vPosition);
    float diffuse = max(dot(normal, lightDirection), 0.0);
    vec3 reflectionDirection = reflect(-lightDirection, normal);
    float specular = diffuse > 0.0
        ? pow(max(dot(viewDirection, reflectionDirection), 0.0), 40.0) : 0.0;
    float attenuation = 1.0 / (1.0 + 0.012 * distanceToLight
                             + 0.0008 * distanceToLight * distanceToLight);
    vec3 surface = uColor * mix(vec3(1.0), texture(uTexture, vUv).rgb, uTextureAmount);
    vec3 ambient = 0.46 * surface;
    vec3 lit = attenuation * (0.86 * diffuse * surface + vec3(0.36 * specular));
    vec3 emission = uEmission * surface;
    fragmentColor = vec4(ambient + lit + emission, uAlpha);
}
