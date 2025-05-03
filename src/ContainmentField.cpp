#include "ContainmentField.h"
#include "Config.h"
#include <cmath>

ContainmentField::ContainmentField(const Config& cfg)
  : halfSize(cfg.field_size/2.0),
    fieldStrength(cfg.initial_strength),
    decayRate(cfg.initial_decay_rate),
    fieldData(cfg.field_grid_size * cfg.field_grid_size, 0.0)
{}

double ContainmentField::getHalfSize() const {
    std::lock_guard lock(mtx);
    return halfSize;
}

bool ContainmentField::isParticleContained(const Particle& p) const {
    double x = p.getX(), y = p.getY();
    double limit = halfSize;
    return (x >= -limit && x <= limit && y >= -limit && y <= limit);
}

void ContainmentField::getContainmentForce(const Particle& p, double& fx, double& fy) const {
    double x = p.getX(), y = p.getY();
    std::lock_guard lock(mtx);
    // distance from center
    double dx = x;
    double dy = y;
    // if at center, no force
    if (dx == 0 && dy == 0) { fx = fy = 0; return; }
    // normalized direction back to center
    double invLen = 1.0 / std::sqrt(dx*dx + dy*dy);
    // magnitude grows linearly to fieldStrength at boundary
    double mag = fieldStrength * (std::sqrt(dx*dx+dy*dy) / halfSize);
    fx = -dx * invLen * mag;
    fy = -dy * invLen * mag;
}

void ContainmentField::setFieldStrength(double s) {
    std::lock_guard lock(mtx);
    fieldStrength = s;
}
double ContainmentField::getFieldStrength() const {
    std::lock_guard lock(mtx);
    return fieldStrength;
}

void ContainmentField::update(double dt) {
    std::lock_guard lock(mtx);
    for (auto& cell : fieldData)
        cell *= (1.0 - decayRate*dt);
    // optional: expire pulses
    for (auto& p : energyPulses)
        p.lifetime -= dt;
    energyPulses.erase(
      std::remove_if(energyPulses.begin(), energyPulses.end(),
                     [](auto& pul){ return pul.lifetime <= 0; }),
      energyPulses.end()
    );
}
