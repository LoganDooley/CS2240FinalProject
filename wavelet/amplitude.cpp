#include "amplitude.h"

unsigned int Amplitude::getResolution(Parameter p){
    return m_resolution[p];
}

void Amplitude::resize(glm::uvec4 newResolution){
    m_resolution = newResolution;
    m_data.resize(m_resolution[Parameter::X] * m_resolution[Parameter::Y] * m_resolution[Parameter::THETA] * m_resolution[Parameter::K]);
}

float& Amplitude::operator()(glm::uvec4 index){
    return m_data[dataIndex(index)];
}

float const &Amplitude::operator()(glm::uvec4 index) const {
    return m_data[dataIndex(index)];
}

void Amplitude::setTemporaryData(){
    for(int x = 0; x < m_resolution[Parameter::X]; x++){
        for(int y = 0; y < m_resolution[Parameter::Y]; y++){
            for(int theta = 0; theta < m_resolution[Parameter::THETA]; theta++){
                for(int k = 0; k < m_resolution[Parameter::K]; k++){
                    if(theta == 0 && k == 0){
                        m_data[dataIndex(glm::uvec4(x, y, theta, k))] = 1;
                    }
                    else{
                        m_data[dataIndex(glm::uvec4(x, y, k, theta))] = 0;
                    }
                }
            }
        }
    }
}

unsigned int Amplitude::dataIndex(glm::uvec4 index) const{
    return index[Parameter::X] +
            index[Parameter::Y] * m_resolution[Parameter::X] +
            index[Parameter::THETA] * m_resolution[Parameter::X] * m_resolution[Parameter::Y] +
            index[Parameter::K] * m_resolution[Parameter::X] * m_resolution[Parameter::Y] * m_resolution[Parameter::THETA];
}
