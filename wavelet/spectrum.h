#pragma once

class Spectrum
{
public:
    Spectrum(float windSpeed);
    ~Spectrum();

    float operator()(float k) const;

private:
    float m_windSpeed = 1;
};
