#include <iostream>
#include <vector>
#include <cmath>
#include <random>

class State {
    public:
    std::vector<Particle> particles;

    void print() {
        for(Particle p: particles) {
            print();
        }
    }
};

class Particle {
    public:
    double mass;
    double position[3];
    double velocity[3];

    Particle(double g_mass, double x_pos, double y_pos, double z_pos, double x_vel, double y_vel, double z_vel) {
        mass = g_mass;
        position[0] = x_pos; position[1] = y_pos; position[2] = z_pos;
        velocity[0] = x_vel; position[1] = y_vel; position[2] = z_vel;
    }
};

double random_double(double min, double max) {
    std::uniform_real_distribution<double> unif(min, max);
    std::default_random_engine re;
    return unif(re);
}

State random_initialization(int n_particles) {
    double max = 10000.0;

    State state;

    for(int i = 0; i < n_particles; i++) {
        Particle p(random_double(0, max), random_double(0, max), random_double(0, max), random_double(0, max), random_double(0, max), random_double(0, max), random_double(0, max));
        state.particles.push_back(p);
    }

    return state;
}

