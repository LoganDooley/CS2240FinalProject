cmake_minimum_required(VERSION 3.0)

# Sets project name
project(SmileWave LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Sets C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Turn off GLFW docs, tests, examples
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(${PROJECT_SOURCE_DIR}/External/glfw)
add_subdirectory(${PROJECT_SOURCE_DIR}/External/glad)
#add_subdirectory(${PROJECT_SOURCE_DIR}/External/imgui)
add_subdirectory(${PROJECT_SOURCE_DIR}/External/glm)

# ImGui includes
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/external/stb
    )

# Specifies .cpp and .h files to be passed to the compiler
add_executable(${PROJECT_NAME}
    main.cpp
    window.cpp
    core.cpp
    camera.cpp
    cubemap.cpp
    fullscreenquad.h
    wavelet/waveletgrid.h
    wavelet/amplitude.h
    wavelet/spectrum.h
    wavelet/profilebuffer.h
    wavelet/environment.h
    wavelet/mathutil.h
    wavelet/wavegeometry.h

    window.h
    core.h
    shaderloader.h
    camera.h
    debug.h
    cubemap.h
    fullscreenquad.cpp
    wavelet/waveletgrid.cpp
    wavelet/amplitude.cpp
    wavelet/spectrum.cpp
    wavelet/profilebuffer.cpp
    wavelet/environment.cpp
    wavelet/mathutil.cpp
    wavelet/wavegeometry.cpp


    # IMGUI files
    external/imgui/imconfig.h
    external/imgui/imgui.cpp
    external/imgui/imgui.h
    external/imgui/imgui_demo.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_internal.h
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imstb_rectpack.h
    external/imgui/imstb_textedit.h
    external/imgui/imstb_truetype.h
    external/imgui/backends/imgui_impl_opengl3.cpp
    external/imgui/backends/imgui_impl_opengl3.h
    external/imgui/backends/imgui_impl_opengl3_loader.h
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_glfw.h

    # STB files
    external/stb/stb_image.h
)

# Specifies libraries to be linked (Qt components, glew, etc)
target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
    glad
    glm
)

#file( COPY ${CMAKE_CURRENT_SOURCE_DIR}/Shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )

# Set this flag to silence warnings on Windows
if (MSVC OR MSYS OR MINGW)
  set(CMAKE_CXX_FLAGS "-Wno-volatile")
endif()

# Set this flag to silence warnings on MacOS
if (APPLE)
  set(CMAKE_CXX_FLAGS "-Wno-deprecated-volatile")
endif()
