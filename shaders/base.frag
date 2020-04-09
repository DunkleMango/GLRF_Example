#version 460 core
#define MAX_POINT_LIGHTS 10
#define PI 3.14159265359
struct Light {
	vec3 F0;
	vec3 L;
	vec3 V;
	vec3 N;
	vec3 radiance;
	vec3 albedo;
	float roughness;
	float metallic;
	float ao;
};
struct Material {
	vec3 albedo;
	float roughness;
	float metallic;
  float ao;
	bool useTextureAlbedo;
	bool useTextureNormal;
	bool useTextureRoughness;
	bool useTextureMetallic;
	bool useTextureAo;
	sampler2D textureAlbedo;
	sampler2D textureNormal;
	sampler2D textureRoughness;
	sampler2D textureMetallic;
	sampler2D textureAo;
};
uniform Material material;
uniform uint pointLight_count;
uniform vec3 pointLight_position[MAX_POINT_LIGHTS];
uniform vec3 pointLight_color[MAX_POINT_LIGHTS];
uniform float pointLight_power[MAX_POINT_LIGHTS];
uniform vec3 directionalLight_direction;
uniform float directionalLight_power;
uniform bool useDirectionalLight;

in vec3 interpolated_normal;
in vec3 interpolated_position;
in vec2 interpolated_uv;
in mat3 TBN;
layout (location = 0) out vec4 frag_color;
layout (location = 1) out vec4 bright_color;

vec3 fresnelSchlick(float cosTheta, vec3 F0);
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 renderLightInfluence(Light light);

void main() {
	vec3 albedo = material.albedo;
	float roughness = material.roughness;
	float metallic = material.metallic;
	float ao = material.ao;
	if (material.useTextureAlbedo) albedo = texture(material.textureAlbedo, interpolated_uv).rgb;
	if (material.useTextureRoughness) roughness = texture(material.textureRoughness, interpolated_uv).r;
	if (material.useTextureMetallic) metallic = texture(material.textureMetallic, interpolated_uv).r;
	if (material.useTextureAo) ao = texture(material.textureAo, interpolated_uv).r;

	vec3 N = normalize(interpolated_normal);
	if (material.useTextureNormal) {
		N = normalize(texture(material.textureNormal, interpolated_uv).rgb * 2.0 - 1.0);
		N = normalize(TBN * N);
	}

  vec3 V = normalize(- interpolated_position);

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);

  vec3 Lo = vec3(0.0);

	for (uint i = 0; i < pointLight_count; i++) {
		vec3 L_denormalized = pointLight_position[i] - interpolated_position;
		float L_dist = length(L_denormalized);
		vec3 L = normalize(L_denormalized);
		float attenuation = 1.0 / (L_dist * L_dist * attenuationModifier);
		vec3 radiance = pointLight_color[i] * attenuation * pointLight_power[i];
		Light light;
		light.F0 = F0;
		light.L = L;
		light.V = V;
		light.N = N;
		light.radiance = radiance;
		light.albedo = albedo;
		light.roughness = roughness;
		light.metallic = metallic;
		light.ao = ao;
		Lo += renderLightInfluence(light);
  }

	if (useDirectionalLight) {
		Light light;
		light.F0 = F0;
		light.L = - directionalLight_direction;
		light.V = V;
		light.N = N;
		light.radiance = vec3(1.0) * directionalLight_power;
		light.albedo = albedo;
		light.roughness = roughness;
		light.metallic = metallic;
		light.ao = ao;
		Lo += renderLightInfluence(light);
	}

  vec3 ambient = vec3(0.03) * albedo * ao;
  vec3 color = ambient + Lo;
  frag_color = vec4(color, 1.0);

	float brightness = dot(frag_color.rgb, vec3(0.2126, 0.7152, 0.0722));
  if (brightness > 1.0) {
		bright_color = vec4(frag_color.rgb, 1.0);
	} else {
		bright_color = vec4(0.0, 0.0, 0.0, 1.0);
	}
}

vec3 renderLightInfluence(Light light) {
	vec3 H = normalize(light.V + light.L);

	vec3 F = fresnelSchlick(max(dot(H, light.V), 0.0), light.F0);
	float NDF = DistributionGGX(light.N, H, light.roughness);
	float G   = GeometrySmith(light.N, light.V, light.L, light.roughness);
	vec3 numerator    = NDF * G * F;
	float denominator = 4.0 * max(dot(light.N, light.V), 0.0) * max(dot(light.N, light.L), 0.0);
	vec3 specular     = numerator / max(denominator, 0.001);
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;

	kD *= 1.0 - light.metallic;
	float NdotL = max(dot(light.N, light.L), 0.0);
	return (kD * light.albedo / PI + specular) * light.radiance * NdotL;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float r      = roughness*roughness;
    float r2     = r*r;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float num   = r2;
    float denom = (NdotH2 * (r2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
