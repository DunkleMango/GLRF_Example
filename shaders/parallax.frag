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

struct MaterialProperty4D {
	vec4 value_default;
	bool use_texture;
	sampler2D texture;
};
struct MaterialProperty3D {
	vec3 value_default;
	bool use_texture;
	sampler2D texture;
};
struct MaterialProperty2D {
	vec2 value_default;
	bool use_texture;
	sampler2D texture;
};
struct MaterialProperty1D {
	float value_default;
	bool use_texture;
	sampler2D texture;
};
struct Material {
	MaterialProperty3D albedo;
	MaterialProperty3D normal;
	MaterialProperty1D roughness;
	MaterialProperty1D metallic;
	MaterialProperty1D ao;
	MaterialProperty1D height;
	MaterialProperty1D opacity;
	float height_scale;
};
uniform Material material;

uniform uint pointLight_count;
uniform vec3 pointLight_color[MAX_POINT_LIGHTS];
uniform float pointLight_power[MAX_POINT_LIGHTS];
uniform float directionalLight_power;
uniform vec3 pointLight_position[MAX_POINT_LIGHTS];
uniform vec3 directionalLight_direction;
uniform bool useDirectionalLight;
uniform vec3 camera_position;
uniform vec3 camera_view_dir;

struct VS_OUT {
  vec2 texcoord;
  vec3 P;
  vec3 N;
  mat3 TBN;
  vec3 tangent_point_light_positions[MAX_POINT_LIGHTS];
  vec3 tangent_camera_position;
  vec3 tangent_camera_view_dir;
  vec3 tangent_P;
};
in VS_OUT VS;

layout (location = 0) out vec4 frag_color;
layout (location = 1) out vec4 bright_color;

vec4 getMaterialPropertyValue(MaterialProperty4D property, vec2 uv);
vec3 getMaterialPropertyValue(MaterialProperty3D property, vec2 uv);
vec2 getMaterialPropertyValue(MaterialProperty2D property, vec2 uv);
float getMaterialPropertyValue(MaterialProperty1D property, vec2 uv);
vec3 fresnelSchlick(float cosTheta, vec3 F0);
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 renderLightInfluence(Light light);
float getDepthFromHeightMap(sampler2D height_map, vec2 texcoord);
float normalizeDepth(float depth);
vec2 ParallaxMapping(vec2 uv, vec3 V, int max_layers, out int count);

void main() {
	vec3 pL[MAX_POINT_LIGHTS];
	for (uint i = 0; i < pointLight_count; i++) {
		pL[i] = VS.tangent_point_light_positions[i] - VS.tangent_P;
	}
	vec3 dL = VS.TBN * normalize(directionalLight_direction);
	vec3 N = vec3(0.0, 0.0, 1.0);
	vec3 V = normalize(VS.tangent_camera_position - VS.tangent_P);

	int max_count = 4000;
	int count;
	// vec2 uv_parallax = (material.height.use_texture) ? ParallaxMapping(VS.texcoord, V, max_count, count) : VS.texcoord;
	vec2 uv_parallax = VS.texcoord;

	N = normalize(getMaterialPropertyValue(material.normal, uv_parallax) * 2.0 - 1.0);
	if (!gl_FrontFacing)
	{
		N = -N;
	}
	vec3 albedo 	= getMaterialPropertyValue(material.albedo, uv_parallax);
	float roughness = getMaterialPropertyValue(material.roughness, uv_parallax);
	float metallic 	= getMaterialPropertyValue(material.metallic, uv_parallax);
	float ao 		= getMaterialPropertyValue(material.ao, uv_parallax);

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);

  	vec3 Lo = vec3(0.0);

	for (uint i = 0; i < pointLight_count; ++i) {
		float L_dist = length(pL[i]);
		vec3 L = normalize(pL[i]);
		float attenuation = 1.0 / (L_dist * L_dist);
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
		light.L = -dL;
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
	//color = getMaterialPropertyValue(material.albedo, VS.texcoord);
	//color = vec3(1.0 / float(count) + 0.2);
	//color = vec3(getMaterialPropertyValue(material.height, VS.texcoord));
	//color = getMaterialPropertyValue(material.normal, VS.texcoord);
	//color = vec3(VS.texcoord, 0.0);
	//color = vec3(2 * float(count) / float(max_count));
	//color = material.albedo.use_texture ? glm::vec3(0.5, 0.0, 0.0) : glm::vec3(0.0, 0.0, 0.5) + material.albedo.value_default;
  	frag_color = vec4(color, 1.0);

	float brightness = dot(frag_color.rgb, vec3(0.2126, 0.7152, 0.0722));
	if (brightness > 1.0) {
		bright_color = vec4(frag_color.rgb - vec3(1.0), 1.0);
	} else {
		bright_color = vec4(0.0, 0.0, 0.0, 1.0);
	}
}

vec4 getMaterialPropertyValue(MaterialProperty4D property, vec2 uv) {
	return property.use_texture ? texture(property.texture, uv) : property.value_default;
}

vec3 getMaterialPropertyValue(MaterialProperty3D property, vec2 uv) {
	return property.use_texture ? texture(property.texture, uv).rgb : property.value_default;
}

vec2 getMaterialPropertyValue(MaterialProperty2D property, vec2 uv) {
	return property.use_texture ? texture(property.texture, uv).rg : property.value_default;
}

float getMaterialPropertyValue(MaterialProperty1D property, vec2 uv) {
	return property.use_texture ? texture(property.texture, uv).r : property.value_default;
}

float getDepthFromHeightMap(sampler2D height_map, vec2 texcoord) {
	return 1.0 - texture(height_map, texcoord).r;
}

float normalizeDepth(float depth) {
	return 0.1 * material.height_scale * depth;
}

vec2 ParallaxMapping(vec2 uv, vec3 V, int max_layers, out int count) {
	if (!material.height.use_texture) {
		count = 0;
		return uv;
	}
	const float dist = length(VS.tangent_camera_position - VS.tangent_P);
	const float capped_dist = max(0, dist - 5.0) / 4.0;
	const float dist2 = capped_dist * capped_dist;
	const float ratio = 1.0 / (1.0 + dist2);
	const float num_layers = mix(mix(1, max_layers, ratio), 1, abs(V.z));
	//const float num_layers = max_layers;
	count = 1;
	float layer_depth = 1.0 / num_layers;
	float layer_depth_cur = 0.0;
	vec2 P = V.xy;
	vec2 uv_delta = P / num_layers;

	vec2 uv_cur = uv;
	float depth_cur = normalizeDepth(getDepthFromHeightMap(material.height.texture, uv_cur));

	while(layer_depth_cur < depth_cur) {
		uv_cur -= uv_delta;
		depth_cur = normalizeDepth(getDepthFromHeightMap(material.height.texture, uv_cur));
		layer_depth_cur += layer_depth;
		++count;
	}

	vec2 uv_prev = uv_cur + uv_delta;
	float post_intersection_depth = depth_cur - layer_depth_cur;
	float pre_intersection_depth = normalizeDepth(getDepthFromHeightMap(material.height.texture, uv_prev))
		- layer_depth_cur + layer_depth;
	
	float weight = post_intersection_depth / (post_intersection_depth - pre_intersection_depth);

	return mix(uv_cur, uv_prev, weight);
}

vec3 renderLightInfluence(Light light) {
	vec3 H = normalize(light.V + light.L);

	vec3 F = fresnelSchlick(max(dot(H, light.V), 0.0), light.F0);
	float NDF = DistributionGGX(light.N, H, light.roughness);
	float G   = GeometrySmith(light.N, light.V, light.L, light.roughness);
	float denominator = 4.0 * max(dot(light.N, light.V), 0.0)
		* max(dot(light.N, light.L), 0.0) + 0.0001;
	vec3 specular     = NDF * G * F / max(denominator, 0.001);
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
    float alpha  = roughness * roughness;
    float alpha2 = alpha * alpha;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = NdotH2 * (alpha2 - 1.0) + 1.0;

    return alpha2 / (PI * denom * denom);
}

float GeometrySchlickGGX(float cos_theta, float k) {
    return cos_theta / (cos_theta * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
	float alpha = roughness * roughness;
    float k = sqrt(2 * alpha * alpha / PI);
    float ggx2  = GeometrySchlickGGX(NdotV, k);
    float ggx1  = GeometrySchlickGGX(NdotL, k);

	return ggx1;
    return ggx1 * ggx2;
}