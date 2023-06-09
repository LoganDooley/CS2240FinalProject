#include "wavegeometry.h"
#include "cubemap.h"

#include <iostream>
#include "debug.h"
#include <string>

WaveGeometry::WaveGeometry(glm::vec2 size, unsigned int resolution):
    m_size(size), m_resolution(resolution)
{

    std::vector<std::string> filenames = {"envs/px.png", "envs/nx.png", "envs/py.png", "envs/ny.png", "envs/pz.png", "envs/nz.png"};
    std::vector<GLenum> facesType = {
        GL_TEXTURE_CUBE_MAP_POSITIVE_X,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    };
    skyTexture = std::make_shared<CubeMap>(
        filenames,
        facesType
    );

    m_heightShader = ShaderLoader::createShaderProgram("Shaders/heightEval.vert", "Shaders/heightEval.frag");
    Debug::checkGLError();

    m_waveShader = ShaderLoader::createShaderProgram("Shaders/wave.vert", "Shaders/wave.frag");
    Debug::checkGLError();

    m_textureShader = ShaderLoader::createShaderProgram("Shaders/texture.vert", "Shaders/texture.frag");
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

    glGenTextures(1, &m_heightMap);
    glBindTexture(GL_TEXTURE_2D, m_heightMap);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, resolution, resolution, 0, GL_RED, GL_FLOAT, nullptr);
    Debug::checkGLError();

    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_heightMap, 0);
    glGenRenderbuffers(1, &m_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, resolution, resolution);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

void WaveGeometry::bindHeightMapTexture(){
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_heightMap);
}

void WaveGeometry::unbindHeightMapTexture(){
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void WaveGeometry::precomputeHeightField(std::shared_ptr<ProfileBuffer> profileBuffer){
    glUseProgram(m_heightShader);

    // TODO: Move this to earlier

    glUniform1i(glGetUniformLocation(m_heightShader, "resolution"), m_resolution);
    glUniform1i(glGetUniformLocation(m_heightShader, "pb_resolution"), 512);
    glUniform2f(glGetUniformLocation(m_heightShader, "gridSpacing"), m_size.x/m_resolution, m_size.y/m_resolution);
    glUniform2f(glGetUniformLocation(m_heightShader, "bottomLeft"), -m_size.x/2, -m_size.y/2);
    glUniform1i(glGetUniformLocation(m_heightShader, "thetaResolution"), 160);
    glUniform1i(glGetUniformLocation(m_heightShader, "kResolution"), profileBuffer->getKResolution());
    glUniform1f(glGetUniformLocation(m_heightShader, "windTheta"), 0);
    glUniform1i(glGetUniformLocation(m_heightShader, "backgroundProfileBuffer"), 0);
    glUniform1i(glGetUniformLocation(m_heightShader, "dynamicProfileBuffer"), 1);
    //profileBuffer->bindProfilebufferTexture();
    profileBuffer->bindBackgroundProfileBuffer();
    profileBuffer->bindDynamicProfileBuffer();
    for (int i = 0; i < setting.simulationResolution[2]; i++) {
        std::string prop = "_Amplitudes[" + std::to_string(i) + "]";
        glUniform1i(glGetUniformLocation(m_heightShader, prop.c_str()), 2 + i);
    }
    {
        int p = 0;
        if (simulator) for (auto texture : simulator->getAmplitudeTextures())
            texture->bind(GL_TEXTURE2 + (p++));
    }
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_resolution, m_resolution);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    {
        int p = 0;
        if (simulator) for (auto texture : simulator->getAmplitudeTextures())
            texture->unbind(GL_TEXTURE2 + (p++));
    }
    profileBuffer->unbindBackgroundProfileBuffer();
    profileBuffer->unbindDynamicProfileBuffer();
}

void WaveGeometry::draw(std::shared_ptr<Camera> camera){
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(m_waveShader);

    glActiveTexture(GL_TEXTURE3);
    skyTexture->bind();

    glUniformMatrix4fv(glGetUniformLocation(m_waveShader, "view"), 1, GL_FALSE, glm::value_ptr(camera->getView()));
    glUniformMatrix4fv(glGetUniformLocation(m_waveShader, "projection"), 1, GL_FALSE, glm::value_ptr(camera->getProjection()));
    glUniform2f(glGetUniformLocation(m_waveShader, "lowerLeft"), - m_size.x/2, - m_size.y/2);
    glUniform2f(glGetUniformLocation(m_waveShader, "upperRight"), m_size.x/2, m_size.y/2);
    glUniform1i(glGetUniformLocation(m_waveShader, "Envmap"), 3);

    bindHeightMapTexture();
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, m_numVerts);
    skyTexture->unbind();
    glBindVertexArray(0);
    glUseProgram(0);
}

void WaveGeometry::debugDraw(){
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(m_textureShader);
    bindHeightMapTexture();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(0);
}
