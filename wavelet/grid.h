#pragma once

#include <array>
#include <vector>
#include <cmath>
#include <glm/vec4.hpp>

class Grid {
    public: 
        Grid();

        int getResolution(int dimension);
        void resize(std::array<int, 4> newResolution);

        float &operator()(int xIndex, int yIndex, int kIndex, int thetaIndex);
        float const &operator()(int xIndex, int yIndex, int kIndex, int thetaIndex) const;
    private:
        std::vector<float> _data;
        std::array<int, 4> _resolution;
};

class WaveletGrid {
        // this is pretty nice from the paper's implementation
        const int X = 0, Y = 1, THETA = 2, K = 3;

        constexpr static float tau = 6.28318530718f;
        const float gravity = 9.81;
        const float surfaceTension = 72.8 / 1000; // surface tension of water
        const float minZeta = log(0.03) / log(2);
        const float maxZeta = log(10) / log(2);
        const float size = 50;
    private:
        std::array<int,4> _resolution;
        glm::vec4   _minParam,
                    _maxParam,
                    _pixelParam;

    public:
        /**
         * Create a new wavelet grid, with the specified resolution.
         * The first two values in the resolution represents the spacial resolution
         * for x and y.
         * The last two values represents the frequency resolution, in terms of theta and
         * the wavenumber.
         */
        WaveletGrid(std::array<int, 4> resolution);

        void takeStep(float dt);
        void heightFieldEvaluation(); // please change parameters to this
    private:
        Grid amplitudes;
        Grid amplitudes_nxt;

        /**
         * Obtains a position, in spacial x frequency space, with the specified 
         * table index.
         *
         * @param index the specified index.
         */
        glm::vec4 getPositionAtIndex(std::array<int,4> index);

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
