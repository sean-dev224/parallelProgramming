#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <random>
#include <stdexcept>
#include <chrono>

const double G = 6.674e-11;
const double SOFTENING_FACTOR = 0.0000001;

//math helper functions
std::vector<double> scalar_multiplication(const std::vector<double> &vector, double scalar) {
    std::vector<double> result;
    for(const double& v: vector) {
        result.push_back(v*scalar);
    }
    return result;
}

bool test_scalar_multiplication() {
    std::vector<double> test = {0, 2, 3};
    std::vector<double> result = scalar_multiplication(test, -2.0);

    
    if(result[0] == 0 && result[1] == -4 && result[2] == -6) {
        std::cout<<"test_scalar_multiplication passed\n";
        return true;
    }
    return false;
}

std::vector<double> add_vector(const std::vector<double>& v1, const std::vector<double>& v2) {
    std::vector<double> result;
    for(int i = 0; i < v1.size(); i++) {
        result.push_back(v1[i] + v2[i]);
    }

    return result;
}

bool test_add_vector() {
    std::vector<double> v1 = {1, 2, 3.5};
    std::vector<double> v2 = {2, 0, 6};

    std::vector<double> result = add_vector(v1, v2);

    if(result[0] == 3 && result[1] == 2 && result[2] == 9.5) {
        std::cout<<"test_add_vector passed\n";
        return true;
    }

    return false;
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
    std::vector<double> velocity = {0, 0, 0};
    std::vector<double> force = {0, 0, 0};

    Particle(std::vector<double>& params) {
        mass = params[0];
        position[0] = params[1]; position[1] = params[2]; position[2] = params[3];
        velocity[0] = params[4]; velocity[1] = params[5]; velocity[2] = params[6];
        force[0] = params[7]; force[1] = params[8]; force[2] = params[9]; 
    }

    std::vector<double> calculate_force(const Particle& p2) const {
        double distance = calculate_distance(position, p2.position) + SOFTENING_FACTOR;

        double gravitational_force = (G * mass * p2.mass) / pow(distance, 2);

        std::vector<double> direction = {p2.position[0] - position[0], p2.position[1] - position[1], p2.position[2] - position[2]};
        std::vector<double> normalized_direction = scalar_multiplication(direction, 1/distance);
        std::vector<double> force_vector = scalar_multiplication(normalized_direction, gravitational_force);

        return force_vector;
    }

    void update_position(double delta_t) {
        std::vector<double> acceleration = scalar_multiplication(force, (1 / mass));
        std::vector<double> velocity_diff = scalar_multiplication(acceleration, delta_t);

        //add new velocity
        velocity = add_vector(velocity, velocity_diff);

        //add new position
        std::vector<double> position_diff = scalar_multiplication(velocity, delta_t);
        position = add_vector(position, position_diff);
        //this->print();
    }

    void reset_force() {
        this->force = {0, 0, 0};
    }

    void print() const {
        std::cout<<"Mass: "<<mass<<", Position: ["<<position[0]<<", "<<position[1]<<", "<<position[2]<<"]"<<", Velocity: ["<<velocity[0]<<", "<<velocity[1]<<", "<<velocity[2]<<"]\n";
    }

    std::string write_particle() const {
        std::stringstream ret;
        double params[] = {mass, position[0], position[1], position[2], velocity[0], velocity[1], velocity[2], force[0], force[1], force[2]};
        for(int i = 0; i < (sizeof(params) / sizeof(params[0])); i++) {
            ret << params[i] << "\t";
        }
        return ret.str();
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
                particles[i].force = add_vector(particles[i].force, force_vector);

                //force applied to particle j will be the same, but opposite direction
                std::vector<double> inv_force_vector = scalar_multiplication(force_vector, -1);
                particles[j].force = add_vector(particles[j].force, inv_force_vector);
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

    // void print_particles() {
    //     for(const Particle &p: particles) {
    //         p.print();
    //     }
    // }

    std::string write_state() const {
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


double random_double(double min, double max) {
    std::random_device rd; //pseudorandom number, generates seed for next step
    std::mt19937 gen(rd()); //better pseudorandom number
    std::uniform_real_distribution<> distribution(min, max); //maps randomness to a number within the uniform distribution

    return distribution(gen);
}

State random_initialization(int n_particles) {
    double max = 1000000000.0;

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

    //ensure filestream is open
    if(!f.is_open()) {
        std::cout<<"File did not open successfully, check your input filepath\n";
        exit(0);
    }

    State state;

    int n_particles;
    f >> n_particles;

    for(int i = 0; i < n_particles; i++) {
        const int ENTRIES_PER_PARTICLE = 10;

        //make particle parameter vector
        std::vector<double> params;
        for(int j = 0; j < ENTRIES_PER_PARTICLE; j++){
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

bool is_integer(const std::string& s) {
    for(char c: s) {
        if(!std::isdigit(c)) {
            return false;
        }
    }
    return true;
}

int main(int argc, char* argv[]) {
    if(argc != 6){
        std::cout<<"Use the following arguments to run on command line:\n";
        std::cout<<"Arg 1: either an integer representing the number of particles (for a random initialization), or a path to an initial state (such as solar.tsv)\n";
        std::cout<<"Arg 2: (string) filepath to an output tsv file. The program will create it if it doesn't exist, or overwrite if it does\n";
        std::cout<<"Arg 3: (double) delta T for each step of the simulation\n";
        std::cout<<"Arg 4: (int) number of timesteps for the simulation\n";
        std::cout<<"Arg 5: (int) how frequently to write the state. The simulation will write every n timesteps\n";
        return 0;
    }
    
    //handle command line arguments
    State s;
    if(is_integer(argv[1])) {
        s = random_initialization(std::stoi(argv[1]));
    } else {
        s = file_initialization(argv[1]);
    }
    std::string output_file = argv[2];
    double delta_t = std::stod(argv[3]);
    int n_timesteps = std::stoi(argv[4]);
    int dump_every_n = std::stoi(argv[5]);

    //record time of execution
    namespace chrn = std::chrono;
    auto start = chrn::high_resolution_clock::now();

    run_simulation(s, output_file, delta_t, n_timesteps, dump_every_n);

    auto end = chrn::high_resolution_clock::now();
    auto elapsed_us = chrn::duration_cast<chrn::microseconds>(end - start).count();
    double elapsed_ms = elapsed_us / 1000.0;

    std::cout<<"Execution time: "<<elapsed_ms<<"ms\n";

    return 0;
}

