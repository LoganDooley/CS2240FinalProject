#version 400 core

layout (triangles, invocations = 8) in; 
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec2 uv;
} gs_in[];

out vec2 uv;

void main() {
    // unroll this loop for performance reason
#pragma openNV (unroll all)
    for (int i = 0; i < 3; i++) {
        gl_Position = gl_in[i].gl_Position;
        gl_Layer = gl_InvocationID;
        uv = gs_in[i].uv;
        EmitVertex();
    }

    EndPrimitive();
}
