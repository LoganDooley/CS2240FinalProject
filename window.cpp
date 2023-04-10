#include "window.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

#ifndef IMGUI_DISABLE

Window::Window(){

}

Window::~Window(){
    if(m_coreAllocated){
        delete(m_core);
    }
    if(m_windowAllocated){
        glfwDestroyWindow(m_GLFWwindow);
    }
    if(m_glfwInitialized){
        glfwTerminate();
    }
}

int Window::start(){
    // Testing glfw
    if(!glfwInit()){
        std::cout<<"GLFW init failed :("<<std::endl;
        return -1;
    }
    m_glfwInitialized = true;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

    m_GLFWwindow = glfwCreateWindow(640, 480, "CS1950U Engine", NULL, NULL);
    if (!m_GLFWwindow)
    {
        std::cout<<"Window Creation Failed :("<<std::endl;
        glfwTerminate();
        return -1;
    }
    m_windowAllocated = true;

    glfwMakeContextCurrent(m_GLFWwindow);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    int width, height;
    glfwGetFramebufferSize(m_GLFWwindow, &width, &height);
    glViewport(0, 0, width, height);

    glfwSwapInterval(1);

    // Set up core now that windowing and opengl are set up
    m_core = new Core(width, height);
    if(!m_core){
        return -1;
    }
    m_coreAllocated = true;

    // Stores variable in glfw to reference our m_core object. This allows it to be accessed
    // even in static methods such as keyCallback and windowSizeCallback
    glfwSetWindowUserPointer(m_GLFWwindow, m_core);

    glfwSetKeyCallback(m_GLFWwindow, keyCallback);

    glfwSetMouseButtonCallback(m_GLFWwindow, mouseButtonCallback);

    glfwSetCursorPosCallback(m_GLFWwindow, cursorPosCallback);

    glfwSetScrollCallback(m_GLFWwindow, scrollCallback);

    glfwSetWindowSizeCallback(m_GLFWwindow, windowSizeCallback);

    glfwSetFramebufferSizeCallback(m_GLFWwindow, framebufferSizeCallback);

    glfwSetInputMode(m_GLFWwindow, GLFW_STICKY_KEYS, GLFW_TRUE);
    
    glfwFocusWindow(m_GLFWwindow);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(m_GLFWwindow, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    return 0;
}

int Window::loop(){
    double previous = glfwGetTime();
    while (!glfwWindowShouldClose(m_GLFWwindow))
    {
        glfwPollEvents();
        double current = glfwGetTime();
        int updateResult = m_core->update(current - previous);
        previous = current;
        if(updateResult != 0){
            return updateResult;
        }
        int drawResult = m_core->draw();
        if(drawResult != 0){
            return drawResult;
        }

        // feed inputs to dear imgui, start new frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        // render your GUI
        ImGui::Begin("Editor Window");

        const char* items[] = { "COMBO", "BOX", "SUPREMACY" };
        static const char* current_item = NULL;

        if (ImGui::BeginCombo("##combo", current_item)) // The second parameter is the label previewed before opening the combo.
        {
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

        ImGui::End();

        // Render dear imgui into screen
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(m_GLFWwindow);
    }

    return 0;
}

void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){
    if(key == GLFW_KEY_ESCAPE){
        glfwSetWindowShouldClose(window, true);
    }
    Core* ptr = (Core*)glfwGetWindowUserPointer(window);
    ptr->keyEvent(key, action);
}

void Window::cursorPosCallback(GLFWwindow* window, double xpos, double ypos){
    Core* ptr = (Core*)glfwGetWindowUserPointer(window);
    ptr->mousePosEvent(xpos, ypos);
}

void Window::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods){
    Core* ptr = (Core*)glfwGetWindowUserPointer(window);
    ptr->mouseButtonEvent(button, action);
}

void Window::scrollCallback(GLFWwindow* window, double xoffset, double yoffset){
    Core* ptr = (Core*)glfwGetWindowUserPointer(window);
    ptr->scrollEvent(yoffset);
}

void Window::windowSizeCallback(GLFWwindow* window, int width, int height){
    Core* ptr = (Core*)glfwGetWindowUserPointer(window);
    ptr->windowResizeEvent(width, height);
}

void Window::framebufferSizeCallback(GLFWwindow* window, int width, int height){
    Core* ptr = (Core*)glfwGetWindowUserPointer(window);
    ptr->framebufferResizeEvent(width, height);
}

#endif
