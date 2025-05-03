#include "Simulation.h"
#include "Config.h"
#include <algorithm>
#include <random>
#include <cmath>
#include <iostream>

// Constructor
Simulation::Simulation(const Config& cfg)
  : fieldSize(cfg.field_size),
    timeStep(cfg.time_step),
    numThreads(cfg.initial_threads),
    threadManager(std::make_unique<ThreadManager>(numThreads)),
    containmentField(std::make_unique<ContainmentField>(cfg))
{
    initializeParticles(cfg);
}

// Destructor
Simulation::~Simulation() {
    stop();
}

// Initialize particles with random positions & velocities
void Simulation::initializeParticles(const Config& cfg) {
    std::mt19937 gen(cfg.random_seed ? cfg.random_seed : std::random_device{}());
    std::uniform_real_distribution<> posDist(-fieldSize/2, fieldSize/2);
    std::uniform_real_distribution<> velDist(-1.0, 1.0);

    particles.clear();
    particles.reserve(cfg.num_particles);
    for (size_t i = 0; i < cfg.num_particles; ++i) {
        auto p = std::make_unique<Particle>(
            posDist(gen), posDist(gen),
            cfg.initial_energy,
            cfg.particle_radius
        );
        p->setVelocity(velDist(gen), velDist(gen));
        particles.push_back(std::move(p));
    }
    std::cout << "Initialized " << particles.size() << " particles.\n";
}

// Start the thread pool
void Simulation::start() {
    threadManager->setNumThreads(numThreads);
    threadManager->start();
    std::cout << "Simulation started with " << numThreads << " threads.\n";
}

// Stop the thread pool
void Simulation::stop() {
    threadManager->stop();
    std::cout << "Simulation stopped.\n";
}

// One simulation step
void Simulation::step() {
    removeEscapedParticles();
    applyForces(timeStep);
    updatePositions(timeStep);
    handleCollisions();
}

// Remove particles outside the containment field
void Simulation::removeEscapedParticles() {
    particles.erase(
        std::remove_if(particles.begin(), particles.end(),
            [&](const std::unique_ptr<Particle>& p){
                return !containmentField->isParticleContained(*p);
            }
        ),
        particles.end()
    );
}

// Apply containment forces in parallel
void Simulation::applyForces(double dt) {
    for (size_t i = 0; i < particles.size(); ++i) {
        threadManager->addTask([this, i, dt]() {
            double fx, fy;
            containmentField->getContainmentForce(*particles[i], fx, fy);
            double newVx = particles[i]->getVX() + fx * dt;
            double newVy = particles[i]->getVY() + fy * dt;
            particles[i]->setVelocity(newVx, newVy);
        });
    }
    threadManager->waitForCompletion();
}

// Update particle positions in parallel
void Simulation::updatePositions(double dt) {
    for (size_t i = 0; i < particles.size(); ++i) {
        threadManager->addTask([this, i, dt]() {
            double nx = particles[i]->getX() + particles[i]->getVX() * dt;
            double ny = particles[i]->getY() + particles[i]->getVY() * dt;
            particles[i]->setPosition(nx, ny);
        });
    }
    threadManager->waitForCompletion();
}

// Detect and handle collisions in parallel
void Simulation::handleCollisions() {
    size_t n = particles.size();
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = i + 1; j < n; ++j) {
            threadManager->addTask([this, i, j]() {
                if (particles[i]->isColliding(*particles[j])) {
                    particles[i]->collide(*particles[j]);
                }
            });
        }
    }
    threadManager->waitForCompletion();
}

// Getter: number of live particles
size_t Simulation::getParticleCount() const {
    return particles.size();
}

// Getter: access particles for rendering
const std::vector<std::unique_ptr<Particle>>& Simulation::getParticles() const {
    return particles;
}

// Sum of all particle energies
double Simulation::getTotalEnergy() const {
    double sum = 0.0;
    for (const auto& p : particles) {
        sum += p->getEnergy();
    }
    return sum;
}

// Change thread‐pool size at runtime
void Simulation::setNumThreads(size_t newNumThreads) {
    numThreads = newNumThreads;
    threadManager->setNumThreads(newNumThreads);
}

