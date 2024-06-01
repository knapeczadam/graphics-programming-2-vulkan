#version 450

layout (location = 0) in vec3 in_color;
layout (location = 1) in vec3 in_position;
layout (location = 2) in vec3 in_normal;
layout (location = 3) in vec2 in_uv;
layout (location = 4) in vec3 in_tangent;

layout (location = 0) out vec4 out_color;

struct light
{
    vec4 position; // w : 0.0f for directional light, 1.0f for point light
    vec4 color;    // w : intensity
};

layout (set = 0, binding = 0) uniform global_ubo
{
    mat4 projection;
    mat4 view;
    mat4 inverse_view;
    vec4 ambient_light_color; // w is intensity
    light lights[10];
    int num_lights;
} ubo;

layout (set = 0, binding = 1) uniform sampler2D diffuse_texture;
layout (set = 0, binding = 2) uniform sampler2D normal_texture;
layout (set = 0, binding = 3) uniform sampler2D specular_texture;
layout (set = 0, binding = 4) uniform sampler2D gloss_texture;

#define PI 3.1415926535897932384626433832795

layout (push_constant) uniform Push
{
    mat4 model_matrix;
    mat4 normal_matrix;
    int shading_mode;
    bool use_normal_map;
} push;

vec3 gLightDir = vec3(0.577f, 0.577f, 0.577f);
float gLightIntensity = 1.0f;
float gKD = 7.0f;
float gShininess = 25.0f;
vec3 gAmbientColor = vec3(0.03f);

vec4 shade_pixel(vec3 normal, vec3 tangent, vec3 view_dir, vec3 diffuse_color, vec3 normal_color, vec3 specular_color, float gloss) 
{
    vec3 color = vec3(0);

    // Binormal
    vec3 binormal = cross(normal, tangent);

    // Tangent-space transformation matrix
    mat3 tangent_space = mat3(tangent, binormal, normal);

    // Remap normal from [0, 1] to [-1, 1]
    normal_color = normal_color * 2.0f - vec3(1.0f);

    // Transform normal from tangent-space to world-space
    normal = push.use_normal_map ? tangent_space * normal_color : normal;

    // Light direction
    vec3 light_dir = normalize(gLightDir);

    // Radiance (directional light)
    vec3 radiance = (vec3(1.0f, 1.0f, 1.0f) * gLightIntensity);

    // Observed area
    vec3 observed_area = vec3(clamp(dot(normal, -light_dir), 0.0f, 1.0f));

    // Diffuse lighting
    vec3 diffuse = diffuse_color * gKD / PI;

    // Phong specular lighting
    vec3 reflected_light = reflect(-light_dir, normal);
    float cos_alpha = clamp(dot(reflected_light, -view_dir), 0.0f, 1.0f);
    vec3 phong = specular_color * pow(cos_alpha, gloss * gShininess);

    if (push.shading_mode == 0)
    {
        color = observed_area;
    }
    else if (push.shading_mode == 1)
    {
        color = diffuse * observed_area;
    }
    else if (push.shading_mode == 2)
    {
        color = phong * observed_area;
    }
    else
    {
        color = radiance * (diffuse + phong + gAmbientColor) * observed_area;
    }
    return vec4(color, 1.0);
}

void main()
{
    vec3 camera_pos_world = ubo.inverse_view[3].xyz;
    vec3 view_dir         = normalize(camera_pos_world - in_position);
    
    vec3 diffuse_color = texture(diffuse_texture, in_uv).rgb;
    vec3 normal_color  = texture(normal_texture, in_uv).rgb;
    vec3 specular_color = texture(specular_texture, in_uv).rgb;
    float gloss_color = texture(gloss_texture, in_uv).r;

    out_color = shade_pixel(in_normal, in_tangent, view_dir, diffuse_color, normal_color, specular_color, gloss_color);
}
