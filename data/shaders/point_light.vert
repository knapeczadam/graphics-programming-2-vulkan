#version 450

const vec2 OFFSETS[6] = vec2[](
    vec2(-1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, -1.0),
    vec2(1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, 1.0)
);

layout (location = 0) out vec2 out_offset;

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

void main()
{
    out_offset = OFFSETS[gl_VertexIndex];
    
    vec4 light_in_camera_space    = ubo.view * vec4(push.position.xyz, 1.0f);
    vec4 position_in_camera_space = light_in_camera_space + push.radius * vec4(out_offset, 0.0f, 0.0f);
    
    gl_Position = ubo.projection * position_in_camera_space;
}
