/******************************************************************************
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following
 * conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of Hobu, Inc. or Flaxen Geo Consulting nor the
 *       names of its contributors may be used to endorse or promote
 *       products derived from this software without specific prior
 *       written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 ****************************************************************************/

#pragma once

#include <algorithm>
#include <utility>
#include <vector>

#include <Eigen/Dense>

namespace LidarTrajectory
{

using namespace Eigen;

class SplineFitScalar
{
public:
    template <typename T>
    static T EndPointCubic(const T& rm, const T& vm, const T& rp, const T& vp,
                           const T& t, T* v = nullptr, T* a = nullptr)
    {
        T rs = rp + rm, rd = rp - rm, vs = vp + vm, vd = vp - vm,
          a0 = (4.0 * rs - vd) / 8.0, a1 = (6.0 * rd - vs) / 4.0, a2 = vd / 2.0,
          a3 = -2.0 * rd + vs;
        if (v)
            *v = t * (t * 3.0 * a3 + 2.0 * a2) + a1;
        if (a)
            *a = t * 6.0 * a3 + 2.0 * a2;
        return t * (t * (t * a3 + a2) + a1) + a0;
    }
};

template <int N> class SplineFit
{
public:
    typedef Matrix<double, N, 1> datum;
    int num;
    double tblock, tstart;
    std::vector<datum> r, v;
    std::vector<bool> missing;
    SplineFit(int _num = -1, double _tblock = 1, double _tstart = 0)
        : num(_num), tblock(_tblock), tstart(_tstart),
          r(num + 1) // num+1 to get both endpoints
          ,
          v(num + 1), missing(num + 1) // missing data at this node?
    {
    }
    datum position(double t) const;
    datum position(double t, datum& velocity) const;
    datum position(double t, datum& velocity, datum& acceleration) const;
    // convert time to index + fractional time
    std::pair<int, double> tconvert(double t) const
    {
        int i = std::min(num - 1,
                         std::max(0, int(std::floor((t - tstart) / tblock))));
        double tf = (t - tstart) / tblock - (i + 0.5);
        return std::make_pair(i, tf);
    }
    // Interpolate/extrapolate r/v entries indicated by clamp.  This leaves
    // missing vector untouched so that a clamp constraint can be put on
    // interior missing nodes.
    bool fillmissing(bool linearfit);
};

typedef SplineFit<3> SplineFit3;

template <int N> class AccelJumpConstraint
{
private:
    double _scale;

public:
    AccelJumpConstraint(double tblock = 1) : _scale(2 / (tblock * tblock)) {}
    template <typename T>
    bool operator()(const T* const ra, // N vec for pos at beg
                    const T* const va, // N vec for vel at beg
                    const T* const vb, // N vec for vel at cent
                    const T* const rc, // N vec for pos at end
                    const T* const vc, // N vec for vel at end
                    // N residuals
                    T* residual) const
    {
        // The jump in the acceleration between a-b and b-c is
        //   8/tblock^2 * ((3*(rc-ra) - (vc+va)) / 4 - vb)
        // Letting scale = 2/tblock^2, and setting the jump to zero, we have
        //   scale * (3*(rc-ra) - (vc+va) - 4*vb) = 0
        const T scale = T(_scale);
        for (int i = 0; i < N; ++i)
            residual[i] =
                scale * (3.0 * (rc[i] - ra[i]) - (vc[i] + va[i]) - 4.0 * vb[i]);
        return true;
    }
};

template <int N> class ClampConstraint
{
private:
    double _scale;

public:
    ClampConstraint(double tblock = 1) : _scale(1 / (tblock * tblock * tblock))
    {
    }
    template <typename T>
    bool operator()(const T* const ra, // N vec for pos at beg
                    const T* const va, // N vec for vel at beg
                    const T* const rb, // N vec for pos at cent
                    const T* const rc, // N vec for pos at end
                    const T* const vc, // N vec for vel at end
                    // N residuals
                    T* residual) const
    {
        // The jump in the third derivate between a-b and b-c is
        //  (4*rb - 2*(rc + ra) + (vc - va)) / tblock^3
        const T scale = T(_scale);
        for (int i = 0; i < N; ++i)
            residual[i] =
                scale * (4.0 * rb[i] - 2.0 * (rc[i] + ra[i]) + (vc[i] - va[i]));
        return true;
    }
};

} // namespace LidarTrajectory