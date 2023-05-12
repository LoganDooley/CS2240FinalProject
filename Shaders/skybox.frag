#version 330 core

out vec4 outColor;

in vec3 cubemap_uvw;

uniform samplerCube skybox;
uniform float exposure = 1;

void main() {
    outColor = vec4(texture(skybox, cubemap_uvw).rgb * exposure, 1);
}
