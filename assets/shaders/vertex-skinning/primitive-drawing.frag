# version 460 core

layout(location = 0) out vec4 frag_color;

layout(location = 0) in VS_OUT
{
    int render_joints;
} fs_in;

void main()
{
    vec4 color = vec4(1.0, 0.0, 0.0, 1.0);
    if(fs_in.render_joints == 0)
    {
        color = vec4(1.0, 1.0, 0.0, 1.0); // yellow
    }
    else
    {
        color = vec4(0.03, 0.3, 1.0, 1.0); // blue
    }
    //frag_color = vec4(0.03, 0.3, 1.0, 1.0);
    frag_color = color;
}