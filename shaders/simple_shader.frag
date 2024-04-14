#version 450

layout (location = 0) in vec3 frag_color;
layout (location = 1) in vec3 frag_pos_world;
layout (location = 2) in vec3 frag_normal_world;

layout (location = 0) out vec4 out_color;

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
    vec3 direction_to_light = ubo.light_position - frag_pos_world;
    float attenuation = 1.0f / dot(direction_to_light, direction_to_light); // distance squared

    vec3 light_color = ubo.light_color.rgb * ubo.light_color.w * attenuation;
    vec3 ambient_light = ubo.ambient_light_color.rgb * ubo.ambient_light_color.w;
    vec3 diffuse_light = light_color * max(dot(normalize(frag_normal_world), normalize(direction_to_light)), 0);

    out_color = vec4((diffuse_light + ambient_light) * frag_color, 1.0f);
}
