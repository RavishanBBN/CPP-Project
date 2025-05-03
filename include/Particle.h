#pragma once
#include <mutex>

/// A single particle in the simulation.
class Particle {
public:
    Particle(double x, double y, double energy,
             double radius, double maxEnergy);
    ~Particle();

    // --- position --------------------------------------------------
    double getX() const;
    double getY() const;
    void   setPosition(double x, double y);

    // --- velocity --------------------------------------------------
    double getVX() const;
    double getVY() const;
    void   setVelocity(double vx, double vy);

    // --- energy ----------------------------------------------------
    double getEnergy() const;
    double getMaxEnergy() const;
    void   setEnergy(double e);
    void   addEnergy(double delta);

    // --- geometry --------------------------------------------------
    double getRadius() const { return radius; }

    // --- collisions ------------------------------------------------
    bool   isColliding(const Particle& other) const;
    void   collide(Particle& other);

    Particle(const Particle&)            = delete;
    Particle& operator=(const Particle&) = delete;

private:
    mutable std::mutex mtx;     // protects all mutable members

    // state
    double x{}, y{};
    double vx{}, vy{};
    double energy{};

    // constants
    const double radius;
    const double maxEnergy;
};
