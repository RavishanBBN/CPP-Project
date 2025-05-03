#include "../include/Simulation.h"
#include "../include/Config.h"

#include <random>
#include <algorithm>
#include <iostream>
#include <chrono>

Simulation::Simulation(const Config& cfg)
    : fieldSize(cfg.field_size),
      timeStep(cfg.time_step),
      numThreads(cfg.initial_threads),
      containmentField(std::make_unique<ContainmentField>(cfg)),
      threadManager(std::make_unique<ThreadManager>(cfg.initial_threads))
{
    std::mt19937 rng(cfg.random_seed ? cfg.random_seed
                                     : std::random_device{}());
    std::uniform_real_distribution<> pos(-fieldSize / 2.0, fieldSize / 2.0);
    std::uniform_real_distribution<> vel(-1.0, 1.0);

    particles.reserve(cfg.num_particles);
    for (size_t i = 0; i < cfg.num_particles; ++i) {
        auto p = std::make_unique<Particle>(
            pos(rng), pos(rng),
            cfg.initial_energy,
            cfg.particle_radius,
            cfg.max_energy);
        p->setVelocity(vel(rng), vel(rng));
        particles.emplace_back(std::move(p));
    }
}

Simulation::~Simulation() { stop(); }

// --------------------------------------------------- lifecycle
void Simulation::start() { threadManager->start(); }
void Simulation::stop()  { threadManager->stop();  }

// --------------------------------------------------- outer step
void Simulation::step()
{
    removeEscapedParticles();
    applyForces(timeStep);
    updatePositions(timeStep);
    handleCollisions();
}

// --------------------------------------------------- particle ops
void Simulation::removeEscapedParticles()
{
    particles.erase(
        std::remove_if(particles.begin(), particles.end(),
            [&](const std::unique_ptr<Particle>& p) {
                return !containmentField->isParticleContained(*p);
            }),
        particles.end());
}

void Simulation::applyForces(double dt)
{
    for (size_t i = 0; i < particles.size(); ++i)
        threadManager->addTask([&, i, dt] {
            double fx, fy;
            containmentField->getContainmentForce(*particles[i], fx, fy);
            particles[i]->setVelocity(
                particles[i]->getVX() + fx * dt,
                particles[i]->getVY() + fy * dt);
        });
    threadManager->waitForCompletion();
}

void Simulation::updatePositions(double dt)
{
    for (size_t i = 0; i < particles.size(); ++i)
        threadManager->addTask([&, i, dt] {
            particles[i]->setPosition(
                particles[i]->getX() + particles[i]->getVX() * dt,
                particles[i]->getY() + particles[i]->getVY() * dt);
        });
    threadManager->waitForCompletion();
}

void Simulation::handleCollisions()
{
    const size_t n = particles.size();
    for (size_t i = 0; i < n; ++i)
        for (size_t j = i + 1; j < n; ++j)
            threadManager->addTask([&, i, j] {
                if (particles[i]->isColliding(*particles[j]))
                    particles[i]->collide(*particles[j]);
            });
    threadManager->waitForCompletion();
}

// --------------------------------------------------- queries / control
size_t Simulation::getParticleCount() const { return particles.size(); }
const std::vector<std::unique_ptr<Particle>>&
Simulation::getParticles() const { return particles; }

double Simulation::getTotalEnergy() const
{
    double sum = 0.0;
    for (const auto& p : particles) sum += p->getEnergy();
    return sum;
}

void Simulation::setNumThreads(size_t n)
{
    numThreads = n;
    threadManager->setNumThreads(n);
}
