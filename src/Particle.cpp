#include "Particle.h"
#include <cmath>
#include <algorithm>

Particle::Particle(double x_, double y_, double energy_, double radius_, double maxE)
    : x(x_), y(y_), vx(0.0), vy(0.0),
      energy(std::min(energy_, maxE)),   // clamp initial energy
      radius(radius_),
      maxEnergy(maxE)
{}

Particle::~Particle() = default;

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
    // Always report no more than maxEnergy
    return std::min(energy, maxEnergy);
}

double Particle::getMaxEnergy() const {
    return maxEnergy;
}

void Particle::setEnergy(double e) {
    std::lock_guard lock(mtx);
    energy = std::clamp(e, 0.0, maxEnergy);
}

void Particle::addEnergy(double delta) {
    std::lock_guard lock(mtx);
    energy = std::clamp(energy + delta, 0.0, maxEnergy);
}

bool Particle::isColliding(const Particle& other) const {
    // lock in address order to avoid deadlock
    const Particle *a = this < &other ? this : &other;
    const Particle *b = this < &other ? &other : this;
    std::scoped_lock lock(a->mtx, b->mtx);

    double dx = x - other.x;
    double dy = y - other.y;
    double rsum = radius + other.radius;
    return (dx*dx + dy*dy) < (rsum * rsum);
}

void Particle::collide(Particle& other) {
    std::scoped_lock lock(mtx, other.mtx);
    std::swap(vx, other.vx);
    std::swap(vy, other.vy);
    // Optional energy loss on collision
    energy       = std::clamp(energy * 0.9, 0.0, maxEnergy);
    other.energy = std::clamp(other.energy * 0.9, 0.0, other.maxEnergy);
}
