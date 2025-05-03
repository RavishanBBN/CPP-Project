#pragma once
#include <mutex>

class Particle {
public:
    /// x,y initial position, energy, radius, and maximum allowed energy
    Particle(double x, double y, double energy, double radius, double maxEnergy);
    ~Particle();

    // Position
    double getX() const;
    double getY() const;
    void   setPosition(double x, double y);

    // Velocity
    double getVX() const;
    double getVY() const;
    void   setVelocity(double vx, double vy);

    // Energy
    double getEnergy() const;
    double getMaxEnergy() const;          // ← added for tests
    void   setEnergy(double e);
    void   addEnergy(double delta);

    // Radius
    double getRadius() const { return radius; }

    // Collision
    bool   isColliding(const Particle& other) const;
    void   collide(Particle& other);

private:
    mutable std::mutex mtx;

    double x, y;
    double vx, vy;

    double energy;
    const double radius;
    const double maxEnergy;               // ← store max energy
};
