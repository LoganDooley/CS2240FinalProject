#include "mathutil.h"
#include <cmath>
#include <functional>

namespace Math {

    float interpolate(float t, std::function<float(int)> f) {
        // we use the monotonic cubic interpolation from https://dl.acm.org/doi/pdf/10.1145/383259.383260
        int tk = t;
        // if t lies on an integral point
        if (t == tk) return f(tk);

        // cache these values to prevent calling f too many times, in case it's expensive
        float v[4] = {f(tk-1), f(tk), f(tk+1), f(tk+2)};

        float dk = (v[2] - v[0]) / 2;
        float dkp1 = (v[3] - v[1]) / 2;
        float deltaK = v[2] - v[1];

        bool signedBitDK = std::signbit(dk);
        bool signedBitDKp1 = std::signbit(dkp1);
        bool signedBitdeltaK = std::signbit(deltaK);

        // if delta is 0 or the signed bit of dk, dkp1, and deltaK differs
        // this is used to force monotonicity of f(t) on the interval [tk, tk+1]
        // so that the interpolation is more stable and is less prone to overshooting
        if (!deltaK || (signedBitDK + signedBitDKp1 + signedBitdeltaK) != 3 * signedBitDK) 
            deltaK = dk = dkp1 = 0;


        // table-based approach of evaluating a 3rd order polynomial
        float a[4] = { v[1], dk, 3 * deltaK - 2 * dk - dkp1, dk + dkp1 - deltaK};

        float f_t = 0;

        float dt = 1;
        for (int i = 0; i < 4; i++) {
            dt *= t-tk;
            f_t += a[i] * dt;
        }

        return f_t;
    }

    float interpolate2D(float x, float y, std::function<float(int, int)> f) {
        // we do 16 calls to f in total, and 5 calls to interpolate
        return interpolate(x, [&](int x) -> float {
            return interpolate(y, std::bind(f, x, std::placeholders::_1));
        });
    }
}
