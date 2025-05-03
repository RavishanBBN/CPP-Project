#include "Particle.h"
#include <algorithm>
#include <cmath>

Particle::Particle(double x_, double y_, double e_, double r_, double maxE)
  : x(x_), y(y_), radius(r_), maxEnergy(maxE)
{
    energy = std::clamp(e_, 0.0, maxEnergy);
}

Particle::~Particle() = default;

// ---------- position ----------
double Particle::getX() const { std::lock_guard g(mtx); return x; }
double Particle::getY() const { std::lock_guard g(mtx); return y; }
void Particle::setPosition(double nx, double ny)
{ std::lock_guard g(mtx); x = nx; y = ny; }

// ---------- velocity ----------
double Particle::getVX() const { std::lock_guard g(mtx); return vx; }
double Particle::getVY() const { std::lock_guard g(mtx); return vy; }
void Particle::setVelocity(double nvx, double nvy)
{ std::lock_guard g(mtx); vx = nvx; vy = nvy; }

// ---------- energy ----------
double Particle::getEnergy() const
{ std::lock_guard g(mtx); return std::min(energy, maxEnergy); }

double Particle::getMaxEnergy() const { return maxEnergy; }

void Particle::setEnergy(double e)
{ std::lock_guard g(mtx); energy = std::clamp(e, 0.0, maxEnergy); }

void Particle::addEnergy(double d)
{ std::lock_guard g(mtx); energy = std::clamp(energy + d, 0.0, maxEnergy); }

// ---------- collision ----------
bool Particle::isColliding(const Particle& o) const
{
    const Particle *a = this < &o ? this : &o;
    const Particle *b = this < &o ? &o   : this;
    std::scoped_lock lock(a->mtx, b->mtx);

    double dx = x - o.x, dy = y - o.y;
    double rsum = radius + o.radius;
    return (dx*dx + dy*dy) < (rsum*rsum);
}

void Particle::collide(Particle& o)
{
    std::scoped_lock lock(mtx, o.mtx);
    std::swap(vx, o.vx);
    std::swap(vy, o.vy);
    energy       = std::clamp(energy       * 0.9, 0.0, maxEnergy);
    o.energy     = std::clamp(o.energy     * 0.9, 0.0, o.maxEnergy);
}
