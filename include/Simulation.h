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

    /// Start the worker threads
    void start();
    /// Stop all threads and clean up
    void stop();
    /// Advance the simulation by one time step
    void step();

    /// How many particles are still in the field?
    size_t getParticleCount() const;
    /// Access particles for rendering or inspection
    const std::vector<std::unique_ptr<Particle>>& getParticles() const;
    /// Sum of all particle energies
    double getTotalEnergy() const;

    /// Adjust the number of threads in use at runtime
    void setNumThreads(size_t newNumThreads);

private:
    void initializeParticles(const Config& cfg);
    void removeEscapedParticles();
    void applyForces(double dt);
    void updatePositions(double dt);
    void handleCollisions();

    std::vector<std::unique_ptr<Particle>> particles;
    std::unique_ptr<ContainmentField> containmentField;
    std::unique_ptr<ThreadManager>   threadManager;

    double fieldSize;
    double timeStep;
    size_t numThreads;
};
