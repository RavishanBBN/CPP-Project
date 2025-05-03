#include "Particle.h"
#include <algorithm>
#include <cmath>

Particle::Particle(double x_, double y_, double energy_, double radius_)
  : x(x_), y(y_), vx(0), vy(0), energy(energy_), radius(radius_)
{}

double Particle::getX() const { std::lock_guard l(mtx); return x; }
double Particle::getY() const { std::lock_guard l(mtx); return y; }
void   Particle::setPosition(double nx, double ny) { std::lock_guard l(mtx); x=nx; y=ny; }

double Particle::getVX() const { std::lock_guard l(mtx); return vx; }
double Particle::getVY() const { std::lock_guard l(mtx); return vy; }
void   Particle::setVelocity(double nvx, double nvy) { std::lock_guard l(mtx); vx=nvx; vy=nvy; }

double Particle::getEnergy() const { std::lock_guard l(mtx); return energy; }
void   Particle::setEnergy(double e) { std::lock_guard l(mtx); energy=e; }
void   Particle::addEnergy(double d)  { std::lock_guard l(mtx); energy=std::max(0.0, energy+d); }

bool Particle::isColliding(const Particle& other) const {
    const Particle *a=this,*b=&other;
    if (a> b) std::swap(a,b);
    std::scoped_lock l(a->mtx,b->mtx);
    double dx=a->x-b->x, dy=a->y-b->y;
    double r=a->radius+b->radius;
    return dx*dx+dy*dy < r*r;
}

void Particle::collide(Particle& other) {
    std::scoped_lock l(mtx, other.mtx);
    std::swap(vx, other.vx);
    std::swap(vy, other.vy);
    energy*=0.9; other.energy*=0.9;
}
