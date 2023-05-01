#include "core.h"

#include "External/imgui/imgui.h"
#include "debug.h"
#include "wavelet/simulator.h"
#include "imgui.h"

Core::Core(int width, int height){
    m_shader = ShaderLoader::createShaderProgram("Shaders/wave.vert", "Shaders/wave.frag");
    Debug::checkGLError();

    m_heightShader = ShaderLoader::createShaderProgram("Shaders/heightEval.vert", "Shaders/heightEval.frag");
    Debug::checkGLError();

    m_camera = std::make_shared<Camera>(width, height, glm::vec3(0, 5, -5), glm::vec3(0, -1, 1), glm::vec3(0, 1, 0), 1.f, 0.1f, 100.f);
    Debug::checkGLError();

    m_waveGeometry = std::make_unique<WaveGeometry>(glm::vec2(100, 100), 400);
    Debug::checkGLError();


    std::array<int,4> resolution = {4096, 4096, 8, 4};
    Simulator::GridSettings setting;
    m_simulator = std::make_unique<Simulator>(resolution, setting);
    Debug::checkGLError();

    m_waveGeometry->setAmplitudeTexture(m_simulator->getAmplitudeTexture());
    Debug::checkGLError();

    m_profileBuffer = std::make_shared<ProfileBuffer>(1, 4096, 0.1, 49, 4);
    glEnable(GL_CULL_FACE);

    glEnable(GL_DEPTH_TEST);
    Debug::checkGLError();
    glViewport(0, 0, width, height);
    Debug::checkGLError();
    m_waveletGrid = std::make_shared<WaveletGrid>(glm::vec4(-50, -50, 0, 1), glm::vec4(50, 50, WaveletGrid::tau, 2), glm::uvec4(100, 100, 16, 4));
    //m_waveletGrid->takeStep(0);
    //m_waveGeometry->update(m_waveletGrid);
    m_fullscreenQuad = std::make_shared<FullscreenQuad>();
    m_fullscreenQuad->bind();
}

Core::~Core(){

}

int Core::update(float seconds){
    /*
    m_camera->move(m_keysDown, seconds);
    if (timeSinceLastUpdate += seconds >= 1.0f/FPS) {
        m_waveletGrid->takeStep(seconds);
        m_waveGeometry->update(m_waveletGrid);
        timeSinceLastUpdate = 0;
    }
    */
    m_camera->move(m_keysDown, seconds);

    const char* items[] = { "Wave geometry", "Advection / Diffusion" };
    static const char* current_item = items[0];

    if (ImGui::BeginCombo("visualizer", current_item)) {
        for (int n = 0; n < IM_ARRAYSIZE(items); n++)
        {
            bool is_selected = (current_item == items[n]); // You can store your selection however you want, outside or inside your objects
            if (ImGui::Selectable(items[n], is_selected))
                current_item = items[n];
            if (is_selected)
                ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
        }
        ImGui::EndCombo();
    }

    if (std::distance(items[0], current_item) == 0) {
        m_simulator->takeStep(seconds);
        m_profileBuffer->precomputeGPU(glfwGetTime());

        m_fullscreenQuad->bind();
        m_waveGeometry->precomputeHeightField(m_profileBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, m_FBOSize.x, m_FBOSize.y);
        m_waveGeometry->draw(m_camera);
        //m_waveGeometry->debugDraw();
        //m_profileBuffer->debugDraw();

    } else {
        if (!simulationPaused) {
            m_simulator->takeStep(seconds);
            if (ImGui::Button("Pause"))     simulationPaused = true;
        } else {
            if (ImGui::Button("Unpause"))   simulationPaused = false;
            if (ImGui::Button("Step"))      m_simulator->takeStep(seconds);
        }
        m_simulator->visualize(m_FBOSize);
    }

    return 0;
}

int Core::draw(){
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0, 0, 0, 1);
    return 0;
}

void Core::keyEvent(int key, int action){
    if(action == GLFW_PRESS){
        m_keysDown.insert(key);
    }
    else if(action == GLFW_RELEASE){
        m_keysDown.erase(key);
    }
}

void Core::mousePosEvent(double xpos, double ypos){
    if(m_mouseDown){
        m_camera->rotate(glm::vec2(xpos, ypos) - m_mousePos);
    }
    m_mousePos = glm::vec2(xpos, ypos);
}

void Core::mouseButtonEvent(int button, int action){
    if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS){
        m_mouseDown = true;
    }
    else if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE){
        m_mouseDown = false;
    }
}

void Core::scrollEvent(double distance){
    
}

void Core::framebufferResizeEvent(int width, int height){
    glViewport(0, 0, width, height);
    m_FBOSize = glm::ivec2(width, height);
}

void Core::windowResizeEvent(int width, int height){
    m_camera->resize(width, height);
}
