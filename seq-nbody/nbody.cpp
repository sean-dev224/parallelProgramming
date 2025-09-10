#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <random>
#include <stdexcept>

const double G = 6.674e-11;
const double SOFTENING_FACTOR = 0.0000001;

//math helper functions
std::vector<double> scalar_multiplication(const std::vector<double> &vector, double scalar) {
    std::vector<double> ret;
    for(const double& v: vector) {
        ret.push_back(v*scalar);
    }
    return ret;
}

bool test_scalar_multiplication() {
    std::vector<double> test = {0, 2, 3};
    std::vector<double> result = scalar_multiplication(test, 2.0);

    
    if(result[0] == 0 && result[1] == 4 && result[2] == 6) {
        std::cout<<"test_scalar_multiplication passed\n";
        return true;
    }
    return false;
}

void add_vector(std::vector<double>& v1, std::vector<double>& v2) {
    //adds v2 to v1 in place
    for(int i = 0; i < v1.size(); i++) {
        v1[i] = v1[i] + v2[i];
    }
}

bool test_add_vector() {
    
}

double calculate_distance(const std::vector<double>& v1, const std::vector<double>& v2) {
    if(v1.size() != v2.size()) {
        throw std::invalid_argument("Vectors must have the same length");
    }

    double sum = 0;
    for(int i = 0; i < v1.size(); i++) {
        sum += pow(v1[i] - v2[i], 2);
    }
    return std::sqrt(sum);
}

//class definitions
class Particle {
    public:
    double mass;
    
    //allow access to full vectors and named individual values
    std::vector<double> position = {0, 0, 0};
    double& x = position[0]; double& y = position[1]; double& z = position[2];
    std::vector<double> velocity = {0, 0, 0};
    double& vx = velocity[0]; double& vy = velocity[1]; double& vz = velocity[2];
    std::vector<double> force = {0, 0, 0};
    double& fx = force[0]; double& fy = force[1]; double& fz = force[2];

    Particle(std::vector<double>& params) {
        mass = params[0];
        position[0] = params[1]; position[1] = params[2]; position[2] = params[3];
        velocity[0] = params[4]; velocity[1] = params[5]; velocity[2] = params[6];
        force[0] = params[7]; force[1] = params[8]; force[2] = params[9]; 
    }

    std::vector<double> calculate_force(const Particle& p2) {
        double distance = calculate_distance(position, p2.position) + SOFTENING_FACTOR;

        double gravitational_force = (G * mass * p2.mass) / pow(distance, 2);

        std::vector<double> direction = {position[0] - p2.position[0], position[1] - p2.position[1], position[2] - p2.position[2]};
        std::vector<double> normalized_direction = scalar_multiplication(direction, 1/distance);
        std::vector<double> force_vector = scalar_multiplication(normalized_direction, gravitational_force);

        return force_vector;
    }

    void update_position(double delta_t) {
        std::vector<double> acceleration = scalar_multiplication(force, (1 / mass));
        std::vector<double> velocity_diff = scalar_multiplication(acceleration, delta_t);

        //add new velocity
        add_vector(velocity, velocity_diff);

        //add new position
        std::vector<double> position_diff = scalar_multiplication(velocity, delta_t);
        add_vector(position, position_diff);
    }

    void reset_force() {
        this->force = {0, 0, 0};
    }

    void print() {
        std::cout<<"Mass: "<<mass<<", Position: ["<<x<<", "<<y<<", "<<z<<"]"<<", Velocity: ["<<vx<<", "<<vy<<", "<<vz<<"]\n";
    }

    std::string write_particle() const{
        std::string ret;
        double params[] = {mass, x, y, z, vx, vy, vz, fx, fy, fz};
        for(int i = 0; i < (sizeof(params) / sizeof(params[0])); i++) {
            ret += std::to_string(params[i]) + "\t";
        }
        return ret;
    }
};

class State {
    bool hasBeenDumped = false;

    public:
    std::vector<Particle> particles;
    std::ofstream tsvFile;
    

    void update_all_forces() {
        for(int i = 0; i < particles.size(); i++) {
            for(int j = i+1; j < particles.size(); j++) {
                //force applied to particle i by particle j
                std::vector<double> force_vector = particles[i].calculate_force(particles[j]);
                add_vector(particles[i].force, force_vector);

                //force applied to particle j will be the same, but opposite direction
                std::vector<double> inv_force_vector = scalar_multiplication(force_vector, -1);
                add_vector(particles[j].force, inv_force_vector);
            }
        }
    }

    void update_all_positions(double delta_t) {
        for(Particle &p: particles) {
            p.update_position(delta_t);
        }
    }

    void reset_forces() {
        for(Particle &p: particles) {
            p.reset_force();
        }
    }

    void print_particles() {
        for(Particle &p: particles) {
            p.print();
        }
    }

    std::string write_state() const{
        std::string state_string = std::to_string(particles.size()) + "\t";

        for(const Particle &p: particles) {
            state_string += p.write_particle();
        }

        return state_string;
    }

    void dump_state(std::string tsvFilePath) {
        //if its the first time dumping this state, clear the output file
        if(!hasBeenDumped) {
            tsvFile.open(tsvFilePath, std::ios::out);
            hasBeenDumped = true;
        } else {
            tsvFile.open(tsvFilePath, std::ios::app | std::ios::out);
        }

        tsvFile<<this->write_state()<<"\n";
        tsvFile.close();
    }


};


double random_double(int min, int max) {
    return (std::rand() % (max-min)) + min;
}

State random_initialization(int n_particles) {
    int max = 1000000;

    State state;

    for(int i = 0; i < n_particles; i++) {
        std::vector<double> params;
        for(int i = 0; i < 10; i++){
            params.push_back(random_double(0, max));
        }
        Particle p(params);
        state.particles.push_back(p);
    }

    return state;
}

State file_initialization(std::string filepath) {
    std::ifstream f(filepath);

    State state;

    int n_particles;
    f >> n_particles;

    for(int i = 0; i < n_particles; i++) {

        //make particle parameter vector
        std::vector<double> params;
        for(int i = 0; i < 10; i++){
            double d;
            f >> d;
            params.push_back(d);
        }

        Particle p(params);
        state.particles.push_back(p);
    }

    return state;
}


void run_simulation(State &s, std::string output_filepath, double delta_t, int n_timesteps, int dump_every_n) {
    for(int i = 0; i < n_timesteps; i++) {
        //run simulation step
        s.update_all_forces();
        s.update_all_positions(delta_t);
        s.reset_forces();

        if(i % dump_every_n == 0) {
            s.dump_state(output_filepath);
        }
    }
}


int main(int argc, char* argv[]) {
    std::cout<<test_scalar_multiplication();

    return 0;



    //State s = random_initialization(5);
    State s = file_initialization("solar.tsv");
    //s.dump_state("output.tsv");
    run_simulation(s, "output.tsv", 10.0, 100, 1);
    return 0;
}

