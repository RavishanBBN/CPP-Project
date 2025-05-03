#pragma once
#include <mutex>

class Particle {
public:
    Particle(double x, double y,
             double energy, double radius, double maxEnergy);
    ~Particle() = default;

    // position
    double getX() const;
    double getY() const;
    void   setPosition(double x, double y);

    // velocity
    double getVX() const;
    double getVY() const;
    void   setVelocity(double vx, double vy);

    // energy
    double getEnergy() const;
    double getMaxEnergy() const;
    void   setEnergy(double e);
    void   addEnergy(double delta);

    // collision helpers
    bool   isColliding(const Particle& other) const;
    void   collide(Particle& other);

    double getRadius() const { return radius; }

    Particle(const Particle&)            = delete;
    Particle& operator=(const Particle&) = delete;

private:
    mutable std::mutex mtx;

    double x{}, y{};
    double vx{}, vy{};
    double energy{};

    const double radius;
    const double maxEnergy;
};
