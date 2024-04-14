#version 450

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 uv;

layout(location = 0) out vec3 frag_color;
layout(location = 1) out vec3 frag_pos_world;
layout(location = 2) out vec3 frag_normal_world;

layout (set = 0, binding = 0) uniform global_ubo
{
    mat4 projection;
    mat4 view;
    vec4 ambient_light_color; // w is intensity
    vec3 light_position;
    vec4 light_color;
} ubo;

layout (push_constant) uniform Push 
{
    mat4 model_matrix;
    mat4 normal_matrix;
} push;

void main()
{
    vec4 position_world = push.model_matrix * vec4(position, 1.0f);
    gl_Position = ubo.projection * (ubo.view * position_world);
    
    frag_normal_world = normalize(mat3(push.normal_matrix) * normal);
    frag_pos_world = position_world.xyz;
    frag_color = color;
}