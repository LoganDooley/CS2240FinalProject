#version 330 core

in vec3 worldSpace_pos;

in float height;

in vec3 surfaceNormal;
in vec3 incidentDirection;
in float useShader;

uniform samplerCube envmap;

out vec4 fragColor;

void main() {
    if (useShader > 0) {
        // Tessendorf Watershader (Section 6.3 Page 18)
        vec3 upwelling = vec3(0, 0.2, 0.3);
        vec3 sky = vec3(0.69,0.84,1);
        vec3 air = vec3(0.1,0.1,0.1);
        float nSnell = 1.34;
        float Kdiffuse = 0.91;
        float reflectivity;
        vec3 nI = incidentDirection;
        vec3 nN = surfaceNormal;
        float costhetai = abs(dot(nI, nN));
        float thetai = acos(costhetai);
        float sinthetat = sin(thetai) / nSnell;
        float thetat = asin(sinthetat);
        if (thetai == 0.0) {
            reflectivity = (nSnell - 1) / (nSnell + 1);
            reflectivity = reflectivity * reflectivity;
        } else {
            float fs = sin(thetat - thetai) / sin(thetat + thetai);
            float ts = tan(thetat - thetai) / tan(thetat + thetai);
            reflectivity = 0.5 * ( fs*fs + ts*ts );
        }
        // vec3 dPE = worldSpace_pos - vec3(0,0,0);
        // float dist = length(dPE) * Kdiffuse;
        // dist = exp(-dist);
        // sky = texture(envmap, nN).rgb;
        float dist = .5;
        fragColor = vec4(dist * (reflectivity * sky + (1 - reflectivity) * upwelling)
            + (1 - dist) * air, 1);
    } else {
        float value = worldSpace_pos.y;
        if (value > 0) {
            fragColor = vec4(value, 0, 0, 1);
        } else {
            fragColor = vec4(0, -value, 0, 1);
        }
        fragColor = vec4(vec3(0.5 + 0.5 * value) * vec3(0, 0, 1), 1);
        fragColor.r = height;
    }    
}
