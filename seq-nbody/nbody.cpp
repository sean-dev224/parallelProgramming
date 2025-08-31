#include <iostream>
#include <vector>

class State {
    std::vector<Particle> particles;
};

class Particle {
    public:
    double mass;
    double x, y, z;
    double vx, vy, vz;

    Particle(double mass, double x, double y, double z, double vx 
};

State random_initialization(int n_particles) {
    for(int i = 0; i < n_particles; i++) {

    }
}