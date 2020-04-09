#version 460 core
uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform float exposure;

in vec2 interpolated_uv;

out vec4 frag_color;

void main() {
    const float gamma = 2.2;
    vec3 sceneColor = texture(scene, interpolated_uv).rgb;
    vec3 bloomColor = texture(bloomBlur, interpolated_uv).rgb;
    vec3 color = sceneColor + bloomColor; // additive blending

    // reinhard tone mapping
    color = vec3(1.0) - exp(-color * exposure);
    //color += bloomColor;

    // gamma correction
    color = pow(color, vec3(1.0 / gamma));

    frag_color = vec4(color, 1.0);
}
