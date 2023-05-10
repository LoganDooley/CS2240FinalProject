#version 330 core

out vec4 outColor;

in vec3 cubemap_uvw;

uniform samplerCube Skybox;
uniform float exposure = 1;

void main() {
    outColor = vec4(texture(Skybox, cubemap_uvw).rgb * exposure, 1);
}