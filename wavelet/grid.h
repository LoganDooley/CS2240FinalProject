#pragma once

#include "amplitude.h"
#include "profilebuffer.h"
#include "wavelet/environment.h"

#include <cmath>
#include <glm/glm.hpp>

struct GridSettings {
    float size = 50;
    glm::vec2 k_range = glm::vec2(0.01, 10);
    float initialTime = 100;
};

class WaveletGrid {
        constexpr static float tau = 6.28318530718f;

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

        GridSettings settings;
        Environment m_environment;

        /**
         * Obtains a position, in spacial x frequency space, with the specified 
         * table index.
         *
         * @param index the specified index.
         */
        glm::vec4 getPositionAtIndex(std::array<unsigned int,4> index) const;

        /**
         * Obtains a spacial position with the specified table index
         *
         * @param index the specified index.
         */
        glm::vec2 getPositionAtIndex(std::array<unsigned int, 2> index) const;

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

        /**
         * @brief Returns the wave direction (\hat{k}_b) given some position (and therefore angle).
         *
         * @param position, specifically theta term.
         *
         * see equation 17 in the paper and the following paragraph. Used for advection step.
         */
        glm::vec2 getWaveDirection(glm::vec4 pos) const;

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

        /**
         * @brief Obtain the default ambient amplitude, used as boundary conditions for 
         * the amplitude table calculations
         *
         * @param x the x position.
         * @param y the y position.
         * @param i_theta the angle index.
         * @param i_k the wavenumber index.
         * @return the ambient amplitude of the specified wave frequency at that position.
         * @note that while i_theta and i_k are indices, (x,y) are cartesian coordinates
         */
        float ambientAmplitude(float x, float y, int i_theta, int i_k) const;

        /**
         * @brief Obtain the reflected (position,wavevector) pair correponding to the input (x,k) pair
         * and the environment.
         *
         * @param pos the (x,y,theta,wavenumber) position.
         * @return glm::vec4 the reflected position.
         */
        glm::vec4 getReflected(glm::vec4 pos) const;
};

