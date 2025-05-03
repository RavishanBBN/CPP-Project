#pragma once
#include "Config.h"
#include "Particle.h"
#include "ContainmentField.h"
#include "ThreadManager.h"
#include <vector>
#include <memory>

class Simulation {
public:
    explicit Simulation(const Config& cfg);
    ~Simulation();

    void start();
    void stop();
    void step();

    size_t getParticleCount() const;
    const std::vector<std::unique_ptr<Particle>>& getParticles() const;
    double getTotalEnergy() const;

    void setNumThreads(size_t newNumThreads);

private:
    void removeEscapedParticles();
    void applyForces(double dt);
    void updatePositions(double dt);
    void handleCollisions();

    std::vector<std::unique_ptr<Particle>> particles;
    std::unique_ptr<ContainmentField> containmentField;
    std::unique_ptr<ThreadManager> threadManager;

    double fieldSize;
    double timeStep;
    size_t numThreads;
};
