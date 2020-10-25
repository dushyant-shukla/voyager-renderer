# version 460 core

layout(location = 0) out vec4 frag_color;

layout (location = 0) in VS_OUT
{
    vec3 color;
} fs_in;

void main()
{
    frag_color = vec4(fs_in.color, 1.0);
}