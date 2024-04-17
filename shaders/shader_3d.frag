#version 450

layout (location = 0) in vec3 frag_color;
layout (location = 1) in vec3 frag_pos_world;
layout (location = 2) in vec3 frag_normal_world;

layout (location = 0) out vec4 out_color;

struct point_light
{
    vec4 position; // ignore w
    vec4 color; // w is intensity
};

layout (set = 0, binding = 0) uniform global_ubo
{
    mat4 projection;
    mat4 view;
    vec4 ambient_light_color; // w is intensity
    point_light point_lights[10];
    int num_lights;
} ubo;

layout (push_constant) uniform Push 
{
    mat4 model_matrix;
    mat4 normal_matrix;
} push;

void main()
{
    vec3 diffuse_light = ubo.ambient_light_color.rgb * ubo.ambient_light_color.w;
    vec3 surface_normal = normalize(frag_normal_world);
    
    for (int i = 0; i < ubo.num_lights; ++i)
    {
        point_light light = ubo.point_lights[i];
        vec3 direction_to_light = light.position.xyz - frag_pos_world;
        float attenuation = 1.0f / dot(direction_to_light, direction_to_light); // distance squared
        float cos_angle_incidence = max(dot(surface_normal, normalize(direction_to_light)), 0);
        vec3 intensity = light.color.rgb * light.color.w * attenuation;
        
        diffuse_light += intensity * cos_angle_incidence;
    }

    out_color = vec4(diffuse_light * frag_color, 1.0f);
}
