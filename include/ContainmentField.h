#pragma once
#include <vector>
#include <mutex>
#include "Particle.h"

struct Config;
class ContainmentField {
public:
    explicit ContainmentField(const Config& cfg);

    double getHalfSize() const;
    bool   isParticleContained(const Particle& p) const;
    // Force *toward* center: zero at center, grows linearly toward boundary
    void   getContainmentForce(const Particle& p, double& fx, double& fy) const;

    void   setFieldStrength(double s);
    double getFieldStrength() const;

    void   update(double dt);

private:
    mutable std::mutex mtx;
    double halfSize;       // half‐width of the square
    double fieldStrength;  // max force magnitude at boundary
    double decayRate;

    struct EnergyPulse {
        double x, y, strength, lifetime;
    };
    std::vector<EnergyPulse> energyPulses;
    std::vector<double>      fieldData;  // optional per‐cell data
};
