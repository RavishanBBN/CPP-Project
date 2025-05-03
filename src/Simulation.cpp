// ──────────────────────────────  src/Simulation.cpp  ───────────────────────────
#include "../include/Simulation.h"
#include "../include/Config.h"
#include <random>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <chrono>

using clock_t = std::chrono::steady_clock;

// ─────────────────────────  Ctor / Dtor ────────────────────────────────────────
Simulation::Simulation(const Config& cfg)
    : fieldSize     {cfg.field_size},
      timeStep      {cfg.time_step},
      containmentField{std::make_unique<ContainmentField>(cfg)},
      threadManager {std::make_unique<ThreadManager>(cfg.initial_threads)},
      numThreads    {cfg.initial_threads}
{
    initializeParticles(cfg);
    threadManager->start();
}

Simulation::~Simulation() { stop(); }

// ─────────────────────────  particle helpers  ─────────────────────────────────
void Simulation::initializeParticles(const Config& cfg)
{
    std::mt19937              gen{cfg.random_seed ? cfg.random_seed
                                                  : static_cast<unsigned>(clock_t::now().time_since_epoch().count())};
    std::uniform_real_distribution<> pos(-fieldSize/2.0, fieldSize/2.0);
    std::uniform_real_distribution<> vel(-1.0, 1.0);

    particles.reserve(cfg.num_particles);
    for (size_t i = 0; i < cfg.num_particles; ++i)
    {
        auto p = std::make_unique<Particle>(pos(gen), pos(gen),
                                            cfg.initial_energy,
                                            cfg.particle_radius,
                                            cfg.max_energy);
        p->setVelocity(vel(gen), vel(gen));
        particles.emplace_back(std::move(p));
    }
    std::cout << "Initialised " << particles.size() << " particles\n";
}

void Simulation::addParticle(std::unique_ptr<Particle> particle)
{
    std::lock_guard<std::mutex> lg(particleMutex);
    particles.emplace_back(std::move(particle));
}

void Simulation::removeEscapedParticles()
{
    std::lock_guard<std::mutex> lg(particleMutex);
    particles.erase(std::remove_if(particles.begin(), particles.end(),
                                   [&](const std::unique_ptr<Particle>& p)
                                   {
                                       return !containmentField->isParticleContained(*p);
                                   }),
                    particles.end());
}

// ─────────────────────────  Thread‑pool lifecycle  ────────────────────────────
void Simulation::start()  { running = true; }   // pool already running
void Simulation::stop()
{
    running = false;
    threadManager->waitForCompletion();
    threadManager->stop();
}

// ─────────────────────────  Public getters  ───────────────────────────────────
size_t Simulation::getParticleCount() const
{
    std::lock_guard<std::mutex> lg(particleMutex);
    return particles.size();
}

const std::vector<std::unique_ptr<Particle>>& Simulation::getParticles() const
{
    return particles;   // caller must treat as read‑only
}

double Simulation::getTotalEnergy() const
{
    std::lock_guard<std::mutex> lg(particleMutex);
    double sum = 0.0;
    for (const auto& p : particles) sum += p->getEnergy();
    return sum;
}

void Simulation::setNumThreads(size_t n)
{
    numThreads = n ? n : 1;
    threadManager->setNumThreads(numThreads);
}
size_t Simulation::getNumThreads() const { return numThreads; }

// ─────────────────────────  Simulation main loop  ─────────────────────────────
void Simulation::step()
{
    // 1. schedule physics
    updatePositions(timeStep);
    applyForces(timeStep);
    handleCollisions();

    // 2. clean‑up
    removeEscapedParticles();

    // 3. wait until all scheduled tasks finished before next frame
    threadManager->waitForCompletion();
}

// ─────────────────────────  Private physics helpers  ──────────────────────────
void Simulation::updatePositions(double dt)
{
    std::lock_guard<std::mutex> lg(particleMutex);
    for (auto& p : particles)
        threadManager->addTask([pRaw=p.get(), dt]
        {
            double nx = pRaw->getX() + pRaw->getVX()*dt;
            double ny = pRaw->getY() + pRaw->getVY()*dt;
            pRaw->setPosition(nx, ny);
        });
}

void Simulation::applyForces(double dt)
{
    std::lock_guard<std::mutex> lg(particleMutex);
    for (auto& p : particles)
        threadManager->addTask([this, pRaw=p.get(), dt]
        {
            // simple radial spring‑like force toward centre
            double x = pRaw->getX();
            double y = pRaw->getY();
            double dist = std::hypot(x, y);
            if (dist < 1e-6) return;     // avoid div‑by‑zero

            double k   = containmentField->getContainmentForce(*pRaw); // magnitude
            double fx  = -k * x / dist;   // normalised vector
            double fy  = -k * y / dist;

            double vx = pRaw->getVX() + fx*dt;
            double vy = pRaw->getVY() + fy*dt;
            pRaw->setVelocity(vx, vy);
        });
}

void Simulation::handleCollisions()
{
    std::lock_guard<std::mutex> lg(particleMutex);

    // naive O(N²) – fine for demo purpose
    for (size_t i = 0; i < particles.size(); ++i)
    for (size_t j = i+1; j < particles.size(); ++j)
    {
        auto* a = particles[i].get();
        auto* b = particles[j].get();
        if (!a->isColliding(*b)) continue;

        threadManager->addTask([a,b]{ a->collide(*b); });
    }
}
