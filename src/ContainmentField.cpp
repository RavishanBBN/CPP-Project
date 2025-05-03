#include "ContainmentField.h"
#include "Config.h"
#include <cmath>

ContainmentField::ContainmentField(const Config& cfg)
 : halfSize(cfg.field_size*0.5),
   fieldStrength(cfg.initial_strength),
   decayRate(cfg.initial_decay_rate)
{}

bool ContainmentField::isParticleContained(const Particle& p) const {
    double x=p.getX(), y=p.getY(), h=halfSize;
    return x>=-h && x<=h && y>=-h && y<=h;
}

void ContainmentField::getContainmentForce(const Particle& p, double& fx, double& fy) const {
    double x=p.getX(), y=p.getY();
    std::lock_guard l(mtx);
    double dist = std::sqrt(x*x+y*y);
    if (dist<1e-8) { fx=fy=0; return; }
    double mag = fieldStrength*(dist/halfSize);
    fx = -x/dist*mag;
    fy = -y/dist*mag;
}
