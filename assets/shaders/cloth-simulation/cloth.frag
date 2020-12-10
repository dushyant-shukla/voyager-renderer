#version 460 core

#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in VS_OUT
{
	vec2 uv;
    vec3 normal;
    vec3 frag_position;
} fs_in;

layout(set = 0, binding = 1) uniform LightUBO
{
    vec3 position;
    vec3 color;
    float constant;
    float linear;
    float quadratic;
} light_ubo;

layout(location = 0) out vec4 frag_color;

float CalculateAttenuation(float D)
{
	float a = light_ubo.constant +
			  light_ubo.linear * D +
			  light_ubo.quadratic * D * D;
	return max(1.0f, a);
}

vec4 CalculateDiffuseComponent(const in vec3 L, const in vec3 N, const in vec4 Kd)
{
	float diffuse_strength = max(dot(N, L), 0.0f);
	return diffuse_strength * Kd;
}

void main() 
{
    vec3 L = normalize(light_ubo.position - fs_in.frag_position);
    vec3 N = normalize(fs_in.normal);
    float attenuation = CalculateAttenuation(length(light_ubo.position - fs_in.frag_position));

	vec4 Kd = vec4(0.03, 0.3, 1.0, 1.0);
    vec4 diffuse_color = CalculateDiffuseComponent(L, N, Kd);

    float ambient_strength = 0.1;
	vec4 ambient_color = ambient_strength * Kd;

    vec4 light_intensity = ((diffuse_color + ambient_color) * vec4(light_ubo.color, 1.0)) / attenuation;

    frag_color = light_intensity;
    //frag_color = Kd;
}