# version 460 core

layout(location = 0) in vec4 position;

layout(set = 0, binding = 0) uniform UBO
{
    mat4 projection;
    mat4 view;
} ubo;

layout (push_constant) uniform ModelData
{
    int is_control_points;
} model_data;

layout (location = 0) out VS_OUT
{
    vec3 color;
} vs_out;

void main()
{
   if(model_data.is_control_points == 1)
   {
        gl_PointSize = 20.0;
        vs_out.color = vec3(1.0, 1.0, 0.0);
   }
   else
   {
        gl_PointSize = 15.0;
        vs_out.color = vec3(1.0, 1.0, 1.0);
   }
   gl_Position = ubo.projection 
                    * ubo.view
                    * position;
} 
