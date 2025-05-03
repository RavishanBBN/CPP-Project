#include "Simulation.h"
#include "Config.h"

#include <random>
#include <algorithm>
#include <cmath>
#include <iostream>

Simulation::Simulation(const Config& cfg)
  : fieldSize(cfg.field_size),
    timeStep(cfg.time_step),
    numThreads(cfg.initial_threads),
    threadManager(std::make_unique<ThreadManager>(numThreads)),
    containmentField(std::make_unique<ContainmentField>(cfg))
{
    std::mt19937 gen(cfg.random_seed ? cfg.random_seed : std::random_device{}());
    std::uniform_real_distribution<> posDist(-fieldSize/2, fieldSize/2);
    std::uniform_real_distribution<> velDist(-1.0, 1.0);

    particles.clear();
    particles.reserve(cfg.num_particles);
    for (size_t i = 0; i < cfg.num_particles; ++i) {
        auto p = std::make_unique<Particle>(
            posDist(gen),                 // x
            posDist(gen),                 // y
            cfg.initial_energy,           // initial energy
            cfg.particle_radius,          // radius
            cfg.max_energy                // ← max energy parameter
        );
        p->setVelocity(velDist(gen), velDist(gen));
        particles.push_back(std::move(p));
    }
    std::cout << "Initialized " << particles.size() << " particles.\n";
}

// ... rest of Simulation.cpp unchanged ...
