#include "Simulation.h"
#include "Config.h"

#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <chrono>
#include <thread>
#include <algorithm>

using json = nlohmann::json;

// Load JSON into Config, including optional random_seed
static Config loadConfig(const std::string& path) {
    std::ifstream f(path);
    if (!f) throw std::runtime_error("Cannot open " + path);
    json j; f >> j;

    Config cfg;
    cfg.num_particles      = j["simulation"]["num_particles"];
    cfg.field_size         = j["simulation"]["field_size"];
    cfg.initial_threads    = j["simulation"]["initial_threads"];
    cfg.time_step          = j["simulation"]["time_step"];

    cfg.initial_energy     = j["particle"]["initial_energy"];
    cfg.max_energy         = j["particle"]["max_energy"];
    cfg.particle_radius    = j["particle"]["radius"];

    cfg.initial_strength   = j["containment_field"]["initial_strength"];
    cfg.initial_decay_rate = j["containment_field"]["initial_decay_rate"];
    cfg.field_grid_size    = j["containment_field"]["grid_size"];

    cfg.target_fps         = j["rendering"]["target_fps"];
    cfg.grid_width         = j["rendering"]["grid_width"];
    cfg.grid_height        = j["rendering"]["grid_height"];
    cfg.max_density_level  = j["rendering"]["max_density_level"];

    cfg.density_map.clear();
    for (auto& [k,v] : j["rendering"]["density_map"].items()) {
        cfg.density_map[std::stoi(k)] = v.get<std::string>()[0];
    }

    if (j["simulation"].contains("random_seed")) {
        cfg.random_seed = j["simulation"]["random_seed"];
    }

    return cfg;
}

// ASCII renderer using map::at() on a const density_map
static void renderASCII(const std::vector<std::unique_ptr<Particle>>& ps,
                        double size, const Config& cfg)
{
    int W = cfg.grid_width;
    int H = cfg.grid_height;
    std::vector<std::vector<int>> grid(H, std::vector<int>(W, 0));

    for (auto& p : ps) {
        int col = std::clamp<int>(
            (p->getX() + size/2) * W / size, 0, W-1);
        int row = std::clamp<int>(
            (p->getY() + size/2) * H / size, 0, H-1);
        grid[row][col]++;
    }

    std::cout << "\033[2J\033[H";               // clear screen
    std::cout << '+' << std::string(W, '-') << "+\n";
    for (int r = 0; r < H; ++r) {
        std::cout << '|';
        for (int c = 0; c < W; ++c) {
            int lvl = std::min(grid[r][c], cfg.max_density_level);
            char ch = (lvl ? cfg.density_map.at(lvl) : ' ');
            std::cout << ch;
        }
        std::cout << "|\n";
    }
    std::cout << '+' << std::string(W, '-') << "+\n";
}

int main() {
    try {
        Config cfg = loadConfig("config.json");
        std::cout << "Configuration loaded.\n";

        Simulation sim(cfg);
        sim.start();

        const double FRAME_TIME = 1.0 / cfg.target_fps;
        int frameCount = 0;

        while (sim.getParticleCount() > 0) {
            auto t0 = std::chrono::high_resolution_clock::now();

            sim.step();
            renderASCII(sim.getParticles(), cfg.field_size, cfg);

            auto dt = std::chrono::duration<double>(
                std::chrono::high_resolution_clock::now() - t0
            ).count();
            if (dt < FRAME_TIME)
                std::this_thread::sleep_for(
                  std::chrono::duration<double>(FRAME_TIME - dt)
                );

            if (++frameCount == 30) {
                frameCount = 0;
                static auto last = t0;
                double fps = 30.0 / std::chrono::duration<double>(
                    std::chrono::high_resolution_clock::now() - last
                ).count();
                last = std::chrono::high_resolution_clock::now();
                std::cout << "Particles: " << sim.getParticleCount()
                          << " | Energy: " << sim.getTotalEnergy()
                          << " | FPS: " << fps << "\n";
            }
        }

        sim.stop();
        std::cout << "Simulation ended. All particles escaped.\n";
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
}
