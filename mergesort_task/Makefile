CFLAGS=-O3 -std=c11 -fPIC -g
CXXFLAGS=-O3 -std=c++17 -fPIC -fopenmp -g
LDFLAGS=-fopenmp
LD=g++


all: mergesort_par


mergesort_par: mergesort_par.o
	$(LD) $(LDFLAGS) mergesort_par.o $(ARCHIVES) -o mergesort_par


bench: mergesort_par
	./queue.sh

clean:
	-rm *.o
	-rm mergesort_par

distclean:
	-rm *.sh.*

.PHONY:  mergesort_par
