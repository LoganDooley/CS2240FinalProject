#pragma once

#include <array>
#include <vector>

class Grid {
    public: 
        Grid();

        float &operator()(int xIndex, int yIndex, int kIndex, int thetaIndex);
        float const &operator()(int xIndex, int yIndex, int kIndex, int thetaIndex) const;
    private:
        std::vector<float> _data;
        std::array<int, 4> _resolution;
};

class WaveletGrid {
        const float gravity = 9.81;
        const float surfaceTension = 72.8 / 1000; // surface tension of water
    public:
        WaveletGrid();

        void takeStep(float dt);
        void heightFieldEvaluation(); // please change parameters to this
    private:
        Grid amplitudes;

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
         * @brief Computes the group speed. This is essentially the derivative of angular
         * frequency with respect to the wavenumber.
         *
         * @param wavenumber this is k in the paper, which is the length of the wavevector bold k
         *
         * see equation 18 in the paper. We use this to compute the diffusion amount
         */
        float groupSpeed(float wavenumber);
};
