#version 460 core

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable


layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 normal;
layout(location = 4) in vec3 tangent;
layout(location = 5) in ivec4 bone_ids;
layout(location = 6) in vec4 bone_weights;

#define MAX_BONES 100

layout(set = 0, binding = 0) uniform ViewUBO
{
    mat4 projection;
    mat4 view;
} view_ubo;

layout(set = 0, binding = 1) uniform BoneUBO
{
    mat4 bones[MAX_BONES];
} bone_ubo;

layout (push_constant) uniform PushModel
{
    mat4 model;
    int enable_animation;
} push_model;

layout (location = 0) out VS_OUT
{
    vec3 color;
    vec2 uv;
    vec3 normal;
    vec3 frag_position;
} vs_out;

void main() 
{
    if(push_model.enable_animation != 0)
    {
        mat4 bone_transform = bone_ubo.bones[bone_ids[0]] * bone_weights[0];
        bone_transform     += bone_ubo.bones[bone_ids[1]] * bone_weights[1];
        bone_transform     += bone_ubo.bones[bone_ids[2]] * bone_weights[2];
        bone_transform     += bone_ubo.bones[bone_ids[3]] * bone_weights[3];
        gl_Position = view_ubo.projection 
                      * view_ubo.view
                      * push_model.model
                      * bone_transform
                      * vec4(position, 1.0);
    }
    else
    {
        gl_Position = view_ubo.projection 
                      * view_ubo.view
                      * push_model.model
                      * vec4(position, 1.0);
    }

    mat3 normal_matrix = mat3(transpose(inverse(push_model.model)));

    vs_out.color = color;
    vs_out.uv = uv;
    vs_out.normal = normal_matrix * normal;
    //vs_out.normal = vec3(push_model.model * vec4(normal, 1.0));
    vs_out.frag_position = vec3(push_model.model * vec4(position, 1.0));
}