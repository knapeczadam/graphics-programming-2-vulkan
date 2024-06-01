#version 450

layout (location = 0) in vec2 in_offset;

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
    mat4 inverse_view;
    vec4 ambient_light_color; // w is intensity
    point_light point_lights[10];
    int num_lights;
} ubo;

layout (push_constant) uniform Push
{
    vec4  position;
    vec4  color;
    float radius;
} push;

#define PI 3.1415926535897932384626433832795

void main()
{
    float dist = sqrt(dot(in_offset, in_offset));
    if (dist >= 1.0f)
    {
        discard;
    }
    
    float cos_dist = 0.5f * (cos(dist * PI) + 1.0f);
    out_color = vec4(push.color.rgb + cos_dist, cos_dist);
}