#version 460 core

#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in VS_OUT
{
    vec3 color;
    vec2 uv;
    vec3 normal;
	int textures_available;
} fs_in;

layout(set = 1, binding = 0) uniform sampler2D diffuse_sampler;
layout(set = 1, binding = 1) uniform sampler2D specular_sampler;
layout(set = 1, binding = 2) uniform sampler2D emissive_sampler;

layout(location = 0) out vec4 frag_color;

void main() 
{
	//if(fs_in.textures_available == 1)
	//{
	    frag_color = texture(diffuse_sampler, fs_in.uv);
	//}
	//else
	//{
	//    frag_color = vec4(fs_in.color, 1.0);
	//}
}