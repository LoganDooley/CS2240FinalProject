#include "wavegeometry.h"

#include <iostream>
#include "debug.h"
#include <string>

WaveGeometry::WaveGeometry(glm::vec2 size, unsigned int resolution):
    m_size(size), m_resolution(resolution)
{
    m_heightShader = ShaderLoader::createShaderProgram("Shaders/heightEval.vert", "Shaders/heightEval.frag");
    Debug::checkGLError();

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
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_DYNAMIC_DRAW);
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<GLvoid*>(0));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    m_numVerts = data.size() / 3;
}

WaveGeometry::~WaveGeometry(){

}

void WaveGeometry::bind(){
    glBindVertexArray(m_vao);
}

void WaveGeometry::unbind(){
    glBindVertexArray(0);
}

int WaveGeometry::getNumVerts(){
    return m_numVerts;
}

void WaveGeometry::update(std::shared_ptr<WaveletGrid> waveletGrid){
    std::vector<float> data;

    glm::vec3 anchor = glm::vec3(-m_size.x/2, 0, -m_size.y/2);
    glm::vec3 xoffset = glm::vec3(-m_size.x/m_resolution, 0, 0);
    glm::vec3 zoffset = glm::vec3(0, 0, -m_size.y/m_resolution);
    for(int i = 0; i<m_resolution; i++){
        anchor.x = -m_size.x/2 + m_size.x/m_resolution * i;
        for(int j = 0; j<m_resolution; j++){
            anchor.z = -m_size.y/2 + m_size.y/m_resolution * j;

            glm::vec3 v1 = anchor;
            glm::vec3 v2 = anchor + xoffset;
            glm::vec3 v3 = anchor + xoffset + zoffset;
            glm::vec3 v4 = anchor + zoffset;

            v1.y = waveletGrid->surfaceAtPoint(glm::vec2(v1.x, v1.z));
            //std::cout<<v1.y<<std::endl;
            v2.y = waveletGrid->surfaceAtPoint(glm::vec2(v2.x, v2.z));
            v3.y = waveletGrid->surfaceAtPoint(glm::vec2(v3.x, v3.z));
            v4.y = waveletGrid->surfaceAtPoint(glm::vec2(v4.x, v4.z));


            Tile t = Tile(v1, v2, v3, v4);
            std::vector<float> tileData = t.unpack();
            data.insert(data.end(), tileData.begin(), tileData.end());
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

}

void WaveGeometry::draw(std::shared_ptr<ProfileBuffer> profileBuffer){
    glUseProgram(m_heightShader);
    glUniform1i(glGetUniformLocation(m_heightShader, "pb_resolution"), 4096);
    std::vector<float> periods = profileBuffer->getPeriods();
    glUniform1fv(glGetUniformLocation(m_heightShader, "periods"), periods.size(), periods.data());
    glUniform2f(glGetUniformLocation(m_heightShader, "gridSpacing"), m_size.x/m_resolution, m_size.y/m_resolution);
    glUniform2f(glGetUniformLocation(m_heightShader, "bottomLeft"), -m_size.x/2, -m_size.y/2);
    glUniform1i(glGetUniformLocation(m_heightShader, "thetaResolution"), 8);
    glUniform1i(glGetUniformLocation(m_heightShader, "zResolution"), 4);
    glViewport(0, 0, m_resolution, m_resolution);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
