CXXFLAGS=-O3
LDFLAGS= -fopenmp

nbody-par: nbody-par.cpp
	g++ $(CXXFLAGS) $(LDFLAGS) nbody-par.cpp -o nbody-par

solar.out: nbody-par
	date
	./nbody-par planet 200 5000000 10000 10 > solar.out # maybe a minutes
	date

solar.pdf: solar.out
	python3 plot.py solar.out solar.pdf 1000 

random.out: nbody-par
	date
	./nbody-par 1000 1 10000 100 10 > random.out # maybe 5 minutes
	date

clean:
	rm nbody-par solar.out solar.pdf random.out