#pragma once
#include <mutex>

class Particle {
public:
    Particle(double x, double y, double energy, double radius);

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
    void   setEnergy(double e);
    void   addEnergy(double delta);

    // Radius accessor
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
};
