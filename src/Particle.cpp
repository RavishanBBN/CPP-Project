#include "Particle.h"
#include <cmath>
#include <algorithm>

Particle::Particle(double x_, double y_, double energy_, double radius_)
  : x(x_), y(y_), vx(0), vy(0), energy(energy_), radius(radius_)
{}

double Particle::getX() const {
    std::lock_guard lock(mtx);
    return x;
}
double Particle::getY() const {
    std::lock_guard lock(mtx);
    return y;
}
void Particle::setPosition(double nx, double ny) {
    std::lock_guard lock(mtx);
    x = nx; y = ny;
}

double Particle::getVX() const {
    std::lock_guard lock(mtx);
    return vx;
}
double Particle::getVY() const {
    std::lock_guard lock(mtx);
    return vy;
}
void Particle::setVelocity(double nvx, double nvy) {
    std::lock_guard lock(mtx);
    vx = nvx; vy = nvy;
}

double Particle::getEnergy() const {
    std::lock_guard lock(mtx);
    return energy;
}
void Particle::setEnergy(double e) {
    std::lock_guard lock(mtx);
    energy = e;
}
void Particle::addEnergy(double delta) {
    std::lock_guard lock(mtx);
    energy = std::max(0.0, energy + delta);
}

bool Particle::isColliding(const Particle& other) const {
    // lock both in address order to avoid deadlock
    const Particle* a = this < &other ? this : &other;
    const Particle* b = this < &other ? &other : this;
    std::scoped_lock lock(a->mtx, b->mtx);

    double dx = x - other.x;
    double dy = y - other.y;
    double dist2 = dx*dx + dy*dy;
    double r = radius + other.radius;
    return dist2 < (r*r);
}

void Particle::collide(Particle& other) {
    // simple elastic swap
    std::scoped_lock lock(mtx, other.mtx);
    std::swap(vx, other.vx);
    std::swap(vy, other.vy);
    // optional energy loss
    energy *= 0.9;
    other.energy *= 0.9;
}
