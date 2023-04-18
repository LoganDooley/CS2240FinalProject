#pragma once

#include "amplitude.h"
#include "profilebuffer.h"

#include <cmath>
#include <glm/glm.hpp>

struct GridSettings {
    float size = 50;
    glm::vec2 k_range = glm::vec2(0.01, 10);
    float initialTime = 100;
};

class WaveletGrid {
        constexpr static float tau = 6.28318530718f;
        GridSettings settings;

        const float gravity = 9.81;
        const float surfaceTension = 72.8 / 1000; // surface tension of water
    private:
        std::array<unsigned int,4> m_resolution;

        // important: max is exclusive
        glm::vec4   m_minParam,
                    m_maxParam,
                    m_unitParam;
        std::vector<ProfileBuffer> m_profileBuffers;

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
        glm::vec4 getPositionAtIndex(std::array<unsigned int,4> index) const;

        void advectionStep(float dt); // see section 4.2 of paper
        void diffusionStep(float dt); // see section 4.2 of paper
        glm::vec3 surfaceAtPoint(glm::vec2 pos) const;

        float idxToPos(const unsigned int idx, Parameter p) const;

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

        // TODO: make inline
        /**
         * @brief Determine if a position is out of bounds of our grid
         *
         * @param pos the position.
         * @return true if out of bounds.
         * @return false otherwise.
         */
        bool outOfBounds(glm::vec2 pos) const;

        /**
         * @brief Remap a position in 2D to indices for x and y, keeping the fractional
         * component
         *
         * @param x the x position.
         * @param y the y position.
         * @return <float,float> the indices with the fractional component.
         */
        std::tuple<float,float> posToIdx(float x, float y) const;
};
