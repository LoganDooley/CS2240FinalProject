#version 330 core

//layout (location = 0) out float value;

out float fragColor;

uniform float t;
uniform float z[8] = {1, 2, 3, 4, 5, 6, 7, 8};
uniform int resolution = 4096;
uniform int periodicity = 2;
uniform int integration_nodes = 100;
uniform float windSpeed;
uniform float unitZ;

float w(float k){
    /* return 1; */
    return sqrt(k * 9.81);
}

float psi(float z){
    float A = pow(1.1, 1.5 * z);
    float B = exp(-1.8038897788076411 * pow(4, z) / pow(windSpeed, 4));

    return 0.139098 * sqrt(A * B);
}

float psiBarIntegrand(float z, float p){
    float waveLength = pow(2, z);
    float waveNumber = 6.28318530718 / waveLength;

    return psi(z) * cos(waveNumber * p - w(waveNumber) * t) * waveLength;
}

float psiBar(float p, int integration_nodes, float z_min, float z_max){
    float dz = (z_max - z_min) / integration_nodes;
    float z = z_min + 0.5 * dz;

    float result = 0;
    for(int i = 0; i<integration_nodes; i++){
        result += psiBarIntegrand(z, p) * dz;
        z += dz;
    }

    return result;
}

void main() {
    int iz = int(floor(gl_FragCoord.y)); // Get row of texture corresponding to z index
    int ip = int(floor(gl_FragCoord.x)); // Get column of texture corresponding to p index

    float z_min = (z[iz] - 0.5) * unitZ;
    float z_max = (z[iz] + 0.5) * unitZ;

    float period = periodicity * pow(2, z_max);
    float p = (ip * period) / resolution;

//value = psiBar(p, integration_nodes, z_min, z_max);
    fragColor = 10 * abs(psiBar(p, integration_nodes, z_min, z_max));
    //fragColor = vec4(vec3(10 * abs(psiBar(p, integration_nodes, z_min, z_max))), 1);
}
