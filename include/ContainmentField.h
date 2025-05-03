#pragma once
#include <mutex>
#include "Particle.h"
struct Config;

class ContainmentField {
public:
    explicit ContainmentField(const Config& cfg);

    bool isParticleContained(const Particle& p) const;
    void getContainmentForce(const Particle& p, double& fx, double& fy) const;

private:
    mutable std::mutex mtx;
    double halfSize;
    double fieldStrength;
    double decayRate;
};
