// --- DataTypes.cpp ---
#include "DataTypes.h"
#include "Config.h"
#include <cmath>
#include <algorithm>

using namespace Eigen;
using namespace std;

Vector3d Node::predictPos(double tk, double t_ref) const {
    if (std::abs(tk - t_ref) < 1e-6 || velocity_curve.empty()) {
        return pos_est;
    }

    double t_start = min(t_ref, tk);
    double t_end = max(t_ref, tk);
    double sign = (tk > t_ref) ? 1.0 : -1.0;

    Vector3d displacement = Vector3d::Zero();

    auto it = velocity_curve.lower_bound(t_start);
    if (it == velocity_curve.end()) return pos_est;

    double prev_t = t_start;
    Vector3d prev_v = it->second;

    for (; it != velocity_curve.end() && it->first <= t_end; ++it) {
        double curr_t = it->first;
        Vector3d curr_v = it->second;

        double dt = curr_t - prev_t;
        displacement += (prev_v + curr_v) / 2.0 * dt;

        prev_t = curr_t;
        prev_v = curr_v;
    }

    if (prev_t < t_end) {
        displacement += prev_v * (t_end - prev_t);
    }

    return pos_est + sign * displacement;
};

Matrix3d  Observation::getWeightMatrix(double unit_sigma) const {
    Matrix3d P = Matrix3d::Zero();
    double s0_sq = unit_sigma * unit_sigma;
    if (type == OBS_RANGING) {
        P(0, 0) = s0_sq / (sigma.x() * sigma.x());
    }
    else {
        P(0, 0) = s0_sq / (sigma.x() * sigma.x());
        P(1, 1) = s0_sq / (sigma.y() * sigma.y());
        P(2, 2) = s0_sq / (sigma.z() * sigma.z());
    }
    return P;
};
