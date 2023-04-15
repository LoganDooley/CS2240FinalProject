#include "amplitude.h"

unsigned int Amplitude::getResolution(Parameter p){
    return m_resolution[p];
}

void Amplitude::resize(std::array<unsigned int, 4> newResolution){
    m_resolution = newResolution;
    m_data.resize(m_resolution[0] * m_resolution[1] * m_resolution[2] * m_resolution[3]);
}

float& Amplitude::operator()(unsigned int xIndex, unsigned int yIndex, unsigned int thetaIndex, unsigned int kIndex){
    return m_data[dataIndex(xIndex, yIndex, kIndex, thetaIndex)];
}

float const &Amplitude::operator()(unsigned int xIndex, unsigned int yIndex, unsigned int thetaIndex, unsigned int kIndex) const {
    return m_data[dataIndex(xIndex, yIndex, kIndex, thetaIndex)];
}

void Amplitude::setTemporaryData(){
    for(int x = 0; x < m_resolution[0]; x++){
        for(int y = 0; y < m_resolution[1]; y++){
            for(int k = 0; k < m_resolution[2]; k++){
                for(int theta = 0; theta < m_resolution[3]; theta++){
                    if(theta == 0 && k == 0){
                        m_data[dataIndex(x, y, k, theta)] = 1;
                    }
                    else{
                        m_data[dataIndex(x, y, k, theta)] = 0;
                    }
                }
            }
        }
    }
}

unsigned int Amplitude::dataIndex(unsigned int xIndex, unsigned int yIndex, unsigned int thetaIndex, unsigned int kIndex) const{
    return xIndex +
            yIndex * m_resolution[0] +
            kIndex * m_resolution[0] * m_resolution[1] +
            thetaIndex * m_resolution[0] * m_resolution[1] * m_resolution[2];
}
