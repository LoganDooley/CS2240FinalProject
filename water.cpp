#include "water.h"
#include <iostream>
#include "debug.h"
#include <string>

Water::Water(glm::vec2 size, unsigned int resolution){
    std::vector<float> data;

    glm::vec3 anchor = glm::vec3(-size.x/2, 0, -size.y/2);
    glm::vec3 xoffset = glm::vec3(-size.x/resolution, 0, 0);
    glm::vec3 zoffset = glm::vec3(0, 0, -size.y/resolution);
    for(int i = 0; i<resolution; i++){
        anchor.x = -size.x/2 + size.x/resolution * i;
        for(int j = 0; j<resolution; j++){
            anchor.z = -size.y/2 + size.y/resolution * j;
            Tile t = Tile(anchor, anchor + xoffset, anchor + xoffset + zoffset, anchor + zoffset);
            std::vector<float> tileData = t.unpack();
            data.insert(data.end(), tileData.begin(), tileData.end());
        }
    }

    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<GLvoid*>(0));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    m_numVerts = data.size() / 3;
}

Water::~Water(){

}

void Water::bind(){
    glBindVertexArray(m_vao);
}

void Water::unbind(){
    glBindVertexArray(0);
}

int Water::getNumVerts(){
    return m_numVerts;
}

void Water::addWave(float wavelength, float steepness, glm::vec2 direction){
    m_waves.emplace_back(Wave(wavelength, steepness, direction));
}

void Water::setWaveUniforms(GLuint shader){
    glUniform1i(glGetUniformLocation(shader, "numWaves"), m_waves.size());
    for(int i = 0; i<m_waves.size(); i++){
        std::string entry = std::string("waves[")+std::to_string(i)+std::string("]");
        glUniform1f(glGetUniformLocation(shader, (entry+std::string(".wavelength")).c_str()), m_waves[i].m_wavelength);
        glUniform1f(glGetUniformLocation(shader, (entry+std::string(".steepness")).c_str()), m_waves[i].m_steepness);
        glUniform1f(glGetUniformLocation(shader, (entry+std::string(".dispersion")).c_str()), m_waves[i].m_dispersion);
        glUniform1f(glGetUniformLocation(shader, (entry+std::string(".wavenumber")).c_str()), m_waves[i].m_wavenumber);
        glUniform1f(glGetUniformLocation(shader, (entry+std::string(".amplitude")).c_str()), m_waves[i].m_amplitude);
        glUniform2f(glGetUniformLocation(shader, (entry+std::string(".direction")).c_str()), m_waves[i].m_direction.x, m_waves[i].m_direction.y);
    }
}