#pragma once

#include "amplitude.h"

#include <cmath>
#include <glm/vec4.hpp>

class WaveletGrid {
        constexpr static float tau = 6.28318530718f;
        const float gravity = 9.81;
        const float surfaceTension = 72.8 / 1000; // surface tension of water
        const float minZeta = log(0.03) / log(2);
        const float maxZeta = log(10) / log(2);
        const float size = 50;
    private:
        std::array<unsigned int,4> m_resolution;
        glm::vec4   m_minParam,
                    m_maxParam,
                    m_pixelParam;

    public:
        /**
         * Create a new wavelet grid, with the specified resolution.
         * The first two values in the resolution represents the spacial resolution
         * for x and y.
         * The last two values represents the frequency resolution, in terms of theta and
         * the wavenumber.
         */
        WaveletGrid(std::array<unsigned int, 4> resolution);

        void takeStep(float dt);
        void heightFieldEvaluation(); // please change parameters to this
    private:
        Amplitude amplitudes;
        Amplitude amplitudes_nxt;

        /**
         * Obtains a position, in spacial x frequency space, with the specified 
         * table index.
         *
         * @param index the specified index.
         */
        glm::vec4 getPositionAtIndex(std::array<unsigned int,4> index);

        void advectionStep(float dt); // see section 4.2 of paper
        void diffusionStep(float dt); // see section 4.2 of paper

        /**
         * @brief Computes the angular frequency at a certain wavenumber.
         *
         * @param wavenumber this is k in the paper, which is the length of the wavevector bold k
         *
         * see angularFrequency on page 4, equation 3
         */
        float angularFrequency(float wavenumber);

        /**
         * @brief Computes the group speed / advection speed. This is essentially the derivative of angular
         * frequency with respect to the wavenumber.
         *
         * @param wavenumber this is k in the paper, which is the length of the wavevector bold k
         *
         * see equation 18 in the paper. We use this to compute the diffusion amount.
         */
        float advectionSpeed(float wavenumber);

        /**
         * @brief Computes the dispersion speed. This is the derivative of the advection speed.
         *
         * @param wavenumber this is k in the paper, which is the length of the wavevector bold k
         *
         * see equation 18 in the paper. We use this to compute the diffusion amount.
         */
        float dispersionSpeed(float wavenumber);
};
